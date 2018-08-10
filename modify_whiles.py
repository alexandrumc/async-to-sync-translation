import random

from pycparser import c_generator
from pycparser.c_ast import *

generator = c_generator.CGenerator()

def test(op):
    if isinstance(op,UnaryOp):
        for i in op:
            if i.name.name == "timeout":
                return True
    return False

def identify_recv_exits(extern_while_body, recv=None):
    for elem in extern_while_body.block_items:
        if isinstance(elem, If):
            if isinstance(elem.cond,UnaryOp):
                if test(elem.cond):
                    elem.cond = UnaryOp("!",recv)

            if elem.coord.line < 0:
                identify_recv_exits(elem.iftrue, elem.cond)
            else:
                identify_recv_exits(elem.iftrue)
                if elem.iffalse:
                    identify_recv_exits(elem.iffalse)


def take_cond_to_break(if_to_check):
    if isinstance(if_to_check.iftrue, Break):
        return if_to_check.cond
    for el in if_to_check.iftrue:
        if isinstance(el, Break):
            if isinstance(if_to_check.cond, FuncCall) and if_to_check.cond.name.name == "timeout":
                pass
            else:
                return if_to_check.cond
        if isinstance(el, If):
            return take_cond_to_break(el)


def take_all_if_to_break(while_to_check):
    conds = []
    for el in while_to_check.stmt:
        aux = None
        if isinstance(el, If):
            aux = take_cond_to_break(el)
        if aux:
            conds.append(aux)
    return conds


def modify_while(while_to_check):
    """
    modifica bucla while,pastreaza doar conditiile din if-ul in care se face break
    :param while_to_check:
    :return:
    """
    conds = take_all_if_to_break(while_to_check)

    aux = conds[0]
    if len(conds) > 1:
        for cond in conds[1:]:
            aux = BinaryOp('||', aux, cond)

    for i in aux:
        if isinstance(i, FuncCall) and i.name.name == "timeout":
            aux = aux.right
    coord = while_to_check.coord
    coord.line = -random.randint(1, 5000)
    new_if = If(aux, None, None, coord)
    return new_if


def to_modify(while_to_check):
    """
    vefifica daca while-ul primit ca parametru indeplineste conditia de a fi modificat:
    sa faca recv + scrie la o variabila de tipul MBOX(numele este mbox)
    intoarce true sau false
    :param while_to_check:
    :return:
    """
    recv = False
    mbox = False
    for line in while_to_check.stmt:
        if isinstance(line, Assignment):
            if isinstance(line.rvalue, FuncCall) and str.lower(line.rvalue.name.name) == "recv":
                recv = True
            left = line.lvalue
            if isinstance(left, ID) and "mbox" in left.name:
                mbox = True
            if isinstance(left, ArrayRef) and isinstance(left.name, StructRef) and "mbox" in left.name.name.name:
                # print "am mbox"
                mbox = True
        elif isinstance(line, If) and isinstance(line.iftrue, Compound):
            for comp in line.iftrue:
                # print type(comp)
                # print type(comp.lvalue.name)
                # if isinstance(comp.lvalue.name)
                if isinstance(comp, Assignment) and isinstance(comp.lvalue.name,
                                                               ID) and "mbox" in comp.lvalue.name.name:
                    mbox = True
                if isinstance(comp, Assignment) and isinstance(comp.lvalue, ArrayRef) and isinstance(comp.lvalue.name,
                                                                                                     StructRef) and \
                        "mbox" in comp.lvalue.name.name.name:
                    mbox = True
    return recv


def whiles_to_if(extern_while_body):
    """
    modifies the main while loop
    all recv loops are translated to ifs

    :param extern_while_body:
    :return: modified main while
    """
    i = 0
    size = len(extern_while_body.block_items)
    list = []
    aux = None
    while i < size:
        aux = extern_while_body
        element = aux.block_items[i]

        if isinstance(element, While) and to_modify(element):
            # test = take_all_if_to_break(element)
            # print len(test)
            coord = element.stmt.coord
            # aux.block_items[i] = modify_while(element)
            new_if = modify_while(element)
            list = aux.block_items[i + 1:]  # next code is part of iftrue
            extern_while_body.block_items[i + 1:] = []  # don't copy the next code
            aux.block_items.remove(aux.block_items[i])
            new_if.iftrue = Compound(list, coord)
            aux.block_items.insert(i, new_if)
            whiles_to_if(new_if.iftrue)

            break
        if isinstance(element, If):
            # if there is any if statement which contains
            # a recv loop in iftrue or iffalse it will be modified

            if isinstance(element.iftrue, Compound):
                for index, item in enumerate(element.iftrue.block_items):

                    if not isinstance(item, While):
                        list.append(item)
                        if isinstance(item, If):
                            whiles_to_if(item.iftrue)  # nu stiu inca de ce trb sa pun asta aici
                    elif to_modify(item):
                        coord = item.stmt.coord

                        new_if = modify_while(item)
                        lista = element.iftrue.block_items[index + 1:]
                        element.iftrue.block_items[index + 1:] = []
                        element.iftrue.block_items.remove(element.iftrue.block_items[index])
                        new_if.iftrue = Compound(lista, coord)
                        element.iftrue.block_items.insert(index, new_if)
                        whiles_to_if(new_if.iftrue)

                        break

            if element.iffalse is not None:
                for index, item in enumerate(element.iffalse.block_items):

                    if not isinstance(item, While):
                        list.append(item)
                        # print item
                    elif to_modify(item):
                        coord = item.stmt.coord
                        new_ifs = modify_while(item)
                        lista = element.iffalse.block_items[index + 1:]
                        element.iffalse.block_items[index + 1:] = []
                        for elem in new_ifs:
                            aux = elem
                            aux.iftrue = Compound(lista, coord)
                            element.iffalse.block_items[index] = aux

                            whiles_to_if(element.iffalse)
                        break

        i += 1
