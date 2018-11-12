from pycparser import c_generator
from pycparser.c_ast import *
from pycparser.plyparser import Coord

generator = c_generator.CGenerator()

coord_aux = 0


def remove_mbox_assign_to_zero(extern_while_body):
    """
    assign mbox = 0 will be removed from the tree
    :param extern_while_body:
    :return:
    """
    to_delete = []
    for elem in extern_while_body.block_items:
        # print elem.coord
        if isinstance(elem, Assignment) and "mbox" in elem.lvalue.name :
            if (isinstance(elem.rvalue,ID) and elem.rvalue.name == "NULL") or int(elem.rvalue.value) == 0 :
                to_delete.append(elem)
        if isinstance(elem, If):
            remove_mbox_assign_to_zero(elem.iftrue)
            if elem.iffalse:
                remove_mbox_assign_to_zero(elem.iffalse)
        # print "aici e ok", elem.coord

    for x in to_delete:
        extern_while_body.block_items.remove(x)


def remove_mbox_free(extern_while_body):
    """
    The if which frees mbox is removed from the tree
    :param extern_while_body:
    :return:
    """
    to_delete = []
    for elem in extern_while_body.block_items:
        if isinstance(elem, If):
            for line in elem.iftrue:
                if isinstance(line, FuncCall) and line.name.name == "free":
                    if line.args.exprs[0].name == "mbox":
                        to_delete.append(elem)
            remove_mbox_free(elem.iftrue)
            if elem.iffalse:
                remove_mbox_free(elem.iffalse)

    for x in to_delete:
        extern_while_body.block_items.remove(x)


def remove_mbox(extern_while_body):
    """
    removes mbox assigns to 0 and ifs where it is freed
    :param extern_while_body:
    :return:
    """
    remove_mbox_assign_to_zero(extern_while_body)
    remove_mbox_free(extern_while_body)


def test(op):
    """
    return True if an operation contains timeout and False otherwise
    :param op:
    :return:
    """
    if isinstance(op, UnaryOp):
        for i in op:
            if i.name.name == "timeout":
                return True
    if isinstance(op, FuncCall):
        if op.name.name == "timeout":
            return True
    return False


def identify_exit_cond(elem, conditions):
    """
    Takes the last condition found with its line > coord.line.

    :param elem:
    :param conditions:
    :return: the last condition found
    """
    aux_cond = None
    for cond, coord in conditions:
        if elem.coord.line > coord.line:
            aux_cond = cond

    return aux_cond


def identify_recv_exits(extern_while_body, conditii):
    """
    Modifies the timeout conditions
    :param extern_while_body:
    :param conditii:
    :return:
    """
    for elem in extern_while_body.block_items:
        if isinstance(elem, If):

            if test(elem.cond):

                aux_cond = identify_exit_cond(elem, conditii)
                if isinstance(elem.cond, UnaryOp) and elem.cond.op == '!':
                    elem.cond = aux_cond
                    # daca e !timeout => fix conditia de iesire
                else:
                    elem.cond = UnaryOp('!', aux_cond, elem.cond.coord)

            else:
                identify_recv_exits(elem.iftrue, conditii)
                if elem.iffalse:
                    identify_recv_exits(elem.iffalse, conditii)


def take_cond_to_break(if_to_check, conds):
    """
    takes the conditions to break from an If
    :param if_to_check:
    :param conds:
    :return:
    """
    if isinstance(if_to_check.iftrue, Break):
        conds.append(if_to_check.cond)
    for el in if_to_check.iftrue:
        if isinstance(el, Break):
            conds.append(if_to_check.cond)
        if isinstance(el, If):
            take_cond_to_break(el, conds)


def take_all_if_to_break(while_to_check):
    """
    gets all conditions to break from a recv loop
    :param while_to_check:
    :return:
    """
    conds = []
    for el in while_to_check.stmt:
        aux_conds = []
        aux = None
        if isinstance(el, If):
            take_cond_to_break(el, aux_conds)
            # print generator.visit(aux)
        if aux_conds:
            for x in aux_conds:
                conds.append(x)
    return conds


def modify_while(while_to_check):
    """
    modifica bucla while,pastreaza doar conditiile din if-ul in care se face break
    :param while_to_check:
    :return:
    """
    global coord_aux
    coord_aux -= 1
    conds = take_all_if_to_break(while_to_check)
    needed_if = True
    aux = conds[0]
    if len(conds) > 1:
        for cond in conds[1:]:
            aux = BinaryOp('||', aux, cond)


    aux, needed_if = remove_timeout_from_cond(aux, needed_if)

    coord = while_to_check.coord
    coord.line = coord_aux

    new_if = If(aux, None, None, coord)
    # new_if.iffalse = iffalse
    if needed_if:
        return new_if
    else:
        return aux


def remove_timeout_from_cond(cond, needed_if):
    """
    removes the timeout from condition and returns the new conditions + needed_if
    :param cond:
    :param needed_if: if it is False, we don't need a new if instead of recv while
    :return:
    """
    if isinstance(cond.left, BinaryOp):
        if isinstance(cond.left.left, FuncCall) and cond.left.left.name.name == "timeout":
            cond.left = cond.left.right
            needed_if = False
        if isinstance(cond.left.right, FuncCall) and cond.left.right.name.name == "timeout":
            cond.left = cond.left.left
            needed_if = False

    if isinstance(cond.right, BinaryOp):
        if isinstance(cond.right.left, FuncCall) and cond.right.left.name.name == "timeout":
            cond.right = cond.right.right
            needed_if = False
        if isinstance(cond.right.right, FuncCall) and cond.right.right.name.name == "timeout":
            cond.right = cond.right.left
            needed_if = False

    if isinstance(cond.left, FuncCall) and cond.left.name.name == "timeout":
        cond = cond.right
        needed_if = False

    if isinstance(cond.right, FuncCall) and cond.right.name.name == "timeout":
        cond = cond.left
        needed_if = False

    if isinstance(cond.left, BinaryOp):
        remove_timeout_from_cond(cond.left, needed_if)
    if isinstance(cond.right, BinaryOp):
        remove_timeout_from_cond(cond.right, needed_if)

    return cond, needed_if


def to_modify(while_to_check):
    """
    vefifica daca while-ul primit ca parametru indeplineste conditia de a fi modificat:
    sa faca recv
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
    global coord_aux
    coord_aux -=1
    i = 0
    # print "aaaaaaaaaaa", extern_while_body.coord

    size = len(extern_while_body.block_items)
    # print "aici e ok"
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

                new_coord = Coord(coord.file, coord.line, coord.column)
                new_coord.line = coord_aux
                new_coord.column = coord_aux

                coord_aux -= 1
                new_break_coord = Coord(coord.file, coord.line, coord.column)
                new_break_coord.line = coord_aux
                new_break_coord.column = coord_aux

                coord_aux -= 1
                id_coord = Coord(coord.file, coord.line, coord.column)
                id_coord.line = coord_aux
                id_coord.column = coord_aux

                coord_aux -= 1
                assign_coord = Coord(coord.file, coord.line, coord.column)
                assign_coord.line = coord_aux
                assign_coord.column = coord_aux

                new_if.iffalse = Compound([Assignment('=', ID("round"), ID("ERR_ROUND"), assign_coord)], new_coord)

                #FuncCall(ID("wait_for_messages", id_coord), None, new_break_coord)

                aux.block_items.insert(i, new_if)
                if new_if.iftrue:
                    whiles_to_if(new_if.iftrue, conditii)

                break
            else:
                delete.append(aux.block_items[i])  # daca are timeout, sterg bucla cu totull
                conditii.append((new_if, coord))
                # aux.block_items[i] = None
        # elif isinstance(element, While) and (not to_modify(element)):
        #         whiles_to_if(element.stmt, conditii)

        if isinstance(element, If):
            # if there is any if statement which contains
            # a recv loop in iftrue or iffalse it will be modified

            if isinstance(element.iftrue, Compound):
                to_delete = []
                for index, item in enumerate(element.iftrue.block_items):

                    if not isinstance(item, While):
                        list.append(item)
                        if isinstance(item, If):
                            if item.iftrue:
                                whiles_to_if(item.iftrue, conditii)  # nu stiu inca de ce trb sa pun asta aici
                    elif to_modify(item):
                        coord = item.stmt.coord

                        new_if = modify_while(item)
                        if isinstance(new_if, If):  # daca intoarce if,adica daca nu are timeout
                            lista = element.iftrue.block_items[index + 1:]
                            element.iftrue.block_items[index + 1:] = []
                            element.iftrue.block_items.remove(element.iftrue.block_items[index])
                            new_if.iftrue = Compound(lista, coord)

                            new_coord = Coord(coord.file, coord.line, coord.column)
                            new_coord.line = coord_aux
                            new_coord.column = coord_aux

                            coord_aux -=1
                            new_break_coord = Coord(coord.file, coord.line, coord.column)
                            new_break_coord.line = coord_aux
                            new_break_coord.column = coord_aux

                            coord_aux -= 1
                            id_coord = Coord(coord.file, coord.line, coord.column)
                            id_coord.line = coord_aux
                            id_coord.column = coord_aux

                            coord_aux -= 1
                            assign_coord = Coord(coord.file, coord.line, coord.column)
                            assign_coord.line = coord_aux
                            assign_coord.column = coord_aux

                            new_if.iffalse = Compound(
                                [
                                 Assignment('=', ID("round"), ID("ERR_ROUND"), assign_coord)], new_coord)

                            element.iftrue.block_items.insert(index, new_if)
                            if new_if.iftrue:
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
                                # new_if.iffalse = Compound([Break()], coord_aux)
                                element.iffalse.block_items.insert(index, new_if)
                                if new_if.iffalse:
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
