import random

from pycparser import c_generator
from pycparser.c_ast import *

generator = c_generator.CGenerator()


def test(op):
    if isinstance(op, UnaryOp):
        for i in op:
            if i.name.name == "timeout":
                return True
    if isinstance(op, FuncCall):
        if op.name.name == "timeout":
            return True
    return False


def identify_exit_cond(elem, conditii):
    aux_cond = None
    for cond, coord in conditii:
        if elem.coord.line > coord.line:
            aux_cond = cond

    return aux_cond
def identify_recv_exits(extern_while_body, conditii):
    for elem in extern_while_body.block_items:
        if isinstance(elem, If):

            if test(elem.cond):

                aux_cond = identify_exit_cond(elem, conditii)
                if isinstance(elem.cond,UnaryOp) and elem.cond.op == '!':
                    elem.cond = aux_cond
                    #daca e !timeout => fix conditia de iesire
                else:
                    elem.cond = UnaryOp('!',aux_cond)

            else:
                identify_recv_exits(elem.iftrue, conditii)
                if elem.iffalse:
                    identify_recv_exits(elem.iffalse, conditii)


def take_cond_to_break(if_to_check):
    if isinstance(if_to_check.iftrue, Break):
        return if_to_check.cond
    for el in if_to_check.iftrue:
        if isinstance(el, Break):
            # if isinstance(if_to_check.cond, FuncCall) and if_to_check.cond.name.name == "timeout":
            #     pass
            # else:
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
    needed_if = True
    aux = conds[0]
    if len(conds) > 1:
        for cond in conds[1:]:
            aux = BinaryOp('||', aux, cond)

    # print generator.visit(aux)
    for i in aux:
        if isinstance(i, FuncCall) and i.name.name == "timeout":
            needed_if = False
    aux = remove_timeout_from_cond(aux)

    coord = while_to_check.coord
    coord.line = -random.randint(1, 5000)

    new_if = If(aux, None, None, coord)
    if needed_if:
        return new_if
    else:
        return aux


def remove_timeout_from_cond(cond):
    if isinstance(cond.left, BinaryOp):
        if isinstance(cond.left.left, FuncCall) and cond.left.left.name.name == "timeout":
            cond.left = cond.left.right
        if isinstance(cond.left.right, FuncCall) and cond.left.right.name.name == "timeout":
            cond.left = cond.left.left

    if isinstance(cond.right, BinaryOp):
        if isinstance(cond.right.left, FuncCall) and cond.right.left.name.name == "timeout":
            cond.right = cond.right.right
        if isinstance(cond.right.right, FuncCall) and cond.right.right.name.name == "timeout":
            cond.right = cond.right.left

    if isinstance(cond.left, FuncCall) and cond.left.name.name == "timeout":
        cond = cond.right

    if isinstance(cond.right, FuncCall) and cond.right.name.name == "timeout":
        cond = cond.left

    if isinstance(cond.left, BinaryOp):
        remove_timeout_from_cond(cond.left)
    if isinstance(cond.right, BinaryOp):
        remove_timeout_from_cond(cond.right)

    return cond


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


def whiles_to_if(extern_while_body, conditii=None):
    """
    modifies the main while loop
    all recv loops are translated to ifs

    :param extern_while_body:
    :return: modified main while
    """
    i = 0
    size = len(extern_while_body.block_items)
    list = []
    delete = []

    while i < size:
        aux = extern_while_body
        element = aux.block_items[i]

        if isinstance(element, While) and to_modify(element):

            coord = element.stmt.coord

            new_if = modify_while(element)
            if isinstance(new_if, If):
                list = aux.block_items[i + 1:]  # next code is part of iftrue
                extern_while_body.block_items[i + 1:] = []  # don't copy the next code
                aux.block_items.remove(aux.block_items[i])
                new_if.iftrue = Compound(list, coord)
                aux.block_items.insert(i, new_if)
                whiles_to_if(new_if.iftrue, conditii)

                break
            else:
                delete.append(aux.block_items[i])  # daca are timeout, sterg bucla cu totull
                conditii.append((new_if, coord))
                # aux.block_items[i] = None
        if isinstance(element, If):
            # if there is any if statement which contains
            # a recv loop in iftrue or iffalse it will be modified

            if isinstance(element.iftrue, Compound):
                to_delete = []
                for index, item in enumerate(element.iftrue.block_items):

                    if not isinstance(item, While):
                        list.append(item)
                        if isinstance(item, If):
                            whiles_to_if(item.iftrue, conditii)  # nu stiu inca de ce trb sa pun asta aici
                    elif to_modify(item):
                        coord = item.stmt.coord

                        new_if = modify_while(item)
                        if isinstance(new_if, If):  # daca intoarce if,adica daca nu are timeout
                            lista = element.iftrue.block_items[index + 1:]
                            element.iftrue.block_items[index + 1:] = []
                            element.iftrue.block_items.remove(element.iftrue.block_items[index])
                            new_if.iftrue = Compound(lista, coord)
                            element.iftrue.block_items.insert(index, new_if)
                            whiles_to_if(new_if.iftrue, conditii)

                            break
                        else:
                            to_delete.append(element.iftrue.block_items[index])
                            conditii.append((new_if, coord))
                            # element.iftrue.block_items[index] = None
                for x in to_delete:
                    element.iftrue.block_items.remove(x)
            if element.iffalse is not None:
                if isinstance(element.iffalse, Compound):
                    to_delete = []
                    for index, item in enumerate(element.iffalse.block_items):

                        if not isinstance(item, While):
                            list.append(item)
                            # print item
                        elif to_modify(item):
                            coord = item.stmt.coord
                            new_if = modify_while(item)
                            if isinstance(new_if, If):
                                lista = element.iffalse.block_items[index + 1:]
                                element.iffalse.block_items[index + 1:] = []
                                element.iffalse.block_items.remove(element.iffalse.block_items[index])
                                new_if.iffalse = Compound(lista, coord)
                                element.iffalse.block_items.insert(index, new_if)
                                whiles_to_if(new_if.iffalse, conditii)

                                break
                            else:
                                to_delete.append(element.iffalse.block_items[index])
                                # element.iffalse.block_items[index] = None
                    for x in to_delete:
                        element.iffalse.block_items.remove(x)

        i += 1
    for x in delete:
        extern_while_body.block_items.remove(x)
