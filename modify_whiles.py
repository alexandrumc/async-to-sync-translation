from pycparser.c_ast import While, If, Assignment, FuncCall, Break, BinaryOp, ArrayRef, StructRef, Compound


def modify_while(while_to_check):
    """
    modifica bucla while,pastreaza doar conditiile din if-ul in care se face break
    :param while_to_check:
    :return:
    """
    line = None
    for line in while_to_check.stmt:
        if (isinstance(line, If)) and isinstance(line.iftrue, Break):
            line.iftrue = None
            if isinstance(line.cond, BinaryOp) and isinstance(line.cond.left,
                                                              FuncCall) and line.cond.left.name.name == "timeout":
                line.cond = line.cond.right
    return line


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
                assignment = line
                # print assignment
                recv = True
            # print line
            left = line.lvalue
            if isinstance(left, ArrayRef) and isinstance(left.name, StructRef) and left.name.name.name == "mbox":
                # print "am mbox"
                mbox = True
        elif isinstance(line, If) and isinstance(line.iftrue, Compound):
            for comp in line.iftrue:
                # print type(comp)
                if isinstance(comp, Assignment) and isinstance(comp.lvalue, ArrayRef) and isinstance(comp.lvalue.name,
                                                                                                     StructRef) and \
                        comp.lvalue.name.name.name == "mbox":
                    mbox = True
    return (mbox and recv)


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
            aux.block_items[i] = modify_while(element)
            list = aux.block_items[i + 1:]  # next code is part of iftrue
            extern_while_body.block_items[i + 1:] = []  # don't copy the next code

            aux.block_items[i].iftrue = Compound(list, element.stmt.coord)  # current iftrue code is all the next code
            whiles_to_if(aux.block_items[i].iftrue)  # apply the function on the ifftrue

            break
        if isinstance(element, If):
            # if there is any if statement which contains
            # a recv loop in iftrue or iffalse it will be modified
            p = len(element.iftrue.block_items)
            # print p
            for index, item in enumerate(element.iftrue.block_items):

                if not isinstance(item, While):
                    list.append(item)
                elif to_modify(item):
                    aux = modify_while(item)
                    aux.iftrue = Compound(element.iftrue.block_items[index + 1:], item.stmt.coord)
                    element.iftrue.block_items[index] = aux
                    element.iftrue.block_items[index + 1:] = []
                    whiles_to_if(element.iftrue)
                    break

            if element.iffalse is not None:
                for index, item in enumerate(element.iffalse.block_items):

                    if not isinstance(item, While):
                        list.append(item)
                    elif to_modify(item):
                        aux = modify_while(item)
                        aux.iftrue = Compound(element.iffalse.block_items[index + 1:], item.stmt.coord)
                        element.iffalse.block_items[index] = aux
                        element.iffalse.block_items[index + 1:] = []
                        whiles_to_if(element.iffalse)
                        break

        i += 1