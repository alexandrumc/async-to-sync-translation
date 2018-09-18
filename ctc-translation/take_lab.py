import copy
import os
from utils import get_label, duplicate_element, get_label_assign_num, generate_c_code_from_paths_and_trees, \
    find_parent, find_node, get_epochs_assigns, find_parentAndrei, get_main_function
from generators import TreeGenerator, RoundGenerator, CheckIfGenerator
from compute_paths import find_all_paths_between_two_nodes
from pycparser import c_generator, parse_file
from pycparser.plyparser import Coord
from pycparser.c_ast import While, Assignment, ID, If, FuncDef, FileAST, UnaryOp, BinaryOp, StructRef, ArrayRef, \
    For, Compound
from modify_whiles import coord_aux

generator = c_generator.CGenerator()

added_vars = []


def conds_to_source_and_dest(current_node, lab_source, lab_dest, destination_reached, source_reached, to_source,
                             to_dest):
    to_delete = []
    for tupleChild in current_node.children():
        child = tupleChild[1]
        if not source_reached:
            if isinstance(child, If) is False:
                if child == lab_source:
                    source_reached.append(True)
                else:
                    to_delete.append(child)
                continue
            else:
                conds_to_source_and_dest(child.iftrue, lab_source, lab_dest, destination_reached, source_reached,
                                         to_source, to_dest)
                if source_reached:
                    child.iffalse = None
                elif child.iffalse is not None:
                    child.iftrue = None
                    conds_to_source_and_dest(child.iffalse, lab_source, lab_dest, destination_reached, source_reached,
                                             to_source, to_dest)
                    to_source.append(child.cond)

                    if not source_reached:
                        child.iffalse = None
                        to_delete.append(child)
                else:
                    to_delete.append(child)
                continue
        else:
            if not destination_reached:
                if isinstance(child, If) is False:
                    if child == lab_dest:
                        destination_reached.append(True)

                    continue
                else:
                    conds_to_source_and_dest(child.iftrue, lab_source, lab_dest, destination_reached, source_reached,
                                             to_source, to_dest)
                    if destination_reached:
                        child.iffalse = None
                        to_dest.append(child)

                    elif child.iffalse is not None:
                        conds_to_source_and_dest(child.iffalse, lab_source, lab_dest, destination_reached,
                                                 source_reached, to_source, to_dest)
                        if destination_reached:
                            child.iftrue = None
                            to_dest.append(child)

                    continue
            else:
                to_delete.append(child)
    for node in to_delete:
        current_node.block_items.remove(node)


def get_extern_while_body_from_func(ast, func_name):
    """
    :param ast:
    :param func_name:
    :return:
    """
    for ext in ast.ext:
        if isinstance(ext, FuncDef) and ext.decl.name == func_name:
            amain_body = ext
            if amain_body is not None:
                for operation in amain_body.body:
                    if isinstance(operation, While):
                        return operation.stmt


def get_extern_while_body(ast):
    if isinstance(ast, FileAST):
        for ext in ast.ext:
            if isinstance(ext, FuncDef) and ext.decl.name == "main":
                main_body = ext.body
                for operation in main_body:
                    if isinstance(operation, While):
                        return operation.stmt
    return ast


def get_labels(filename, labelname):
    """
    appends the label value in the list when it is found, if it is already in the list, it is ignored
    :param filename:
    :param labelname:
    :return: list with labels values
    """
    labels = []
    lab = labelname + "="
    with open(filename) as f:
        lines = f.readlines()
    for each in lines:
        if labelname in each:
            aux = each.replace(" ", "")
            aux = aux[aux.index(labelname):]
            if "\n" in aux:
                aux = aux.replace("\n", "")
            aux = aux.replace(lab, "")
            aux = aux.replace(";", "")
            if aux.endswith("ROUND"):
                if aux not in labels and "old" not in aux:
                    labels.append(aux)
    labels.append("ERR_ROUND")
    return labels


def get_labels_order(filename, labelname):
    """
    similar with get_labels but appends the label value in the list when it is found the last time
    :param filename:
    :param labelname:
    :return: list with label values sorted
    """
    aux_list = []
    labels = []
    lab = labelname + "="
    with open(filename) as f:
        lines = f.readlines()
    for each in lines:
        if labelname in each:
            aux = each.replace(" ", "")
            aux = aux[aux.index(labelname):]
            if "\n" in aux:
                aux = aux.replace("\n", "")
            aux = aux.replace(lab, "")
            aux = aux.replace(";", "")
            if aux.endswith("ROUND"):
                labels.append(aux)

    for i in xrange(len(labels)):
        if labels[i] not in labels[i + 1:]:
            aux_list.append(labels[i])
    aux_list.append("ERR_ROUND")
    return aux_list


def get_context(extern, context):
    if extern:
        for elem in extern.block_items:
            if isinstance(elem, If):
                context.append(elem)
                get_context(elem.iftrue, context)
            else:
                break


def modify_conds(label1, conds_dict, cop, ast):
    if label1 in conds_dict:
        # print label1
        context = []
        get_context(get_extern_while_body(cop), context)
        conds = conds_dict[label1]
        for tuple in conds:
            ifelem = tuple[0]
            new_vals = tuple[1]
            # print new_vals
            node_cop = find_node(cop, ifelem)
            node_ast = find_node(ast, ifelem)
            if node_cop:
                modify_cond(node_cop.cond, new_vals)
            if node_ast:
                modify_cond(node_ast.cond, new_vals)


def get_paths_trees(ast, labels, labels_sorted, labelname):
    trees_dict = {}
    trees_paths_dict = {}
    aux_list = []
    conds_dict = {}
    is_job = False

    for label1 in labels_sorted:

        if label1 == "AUX_ROUND" or label1 == "ERR_ROUND":
            continue

        trees_list = []
        trees_paths_list = []
        labels_start = get_label(ast, labelname, label1)

        for label2 in labels_sorted[labels_sorted.index(label1) + 1:]:

            labels_end = get_label(ast, labelname, label2)
            # print label1, label2
            new_conds = []

            for start in labels_start:
                # la final sa bag conditiile pt urmatorul element de iterat
                for end in labels_end:
                    cop = duplicate_element(ast)
                    # prune_tree(get_extern_while_body_from_func(cop),start,end,[],[])

                    dest_list = []
                    source_list = []
                    # prune_tree(get_extern_while_body(cop), start, end, dest_list, source_list)
                    ifs_to_dest = []
                    conds_to_source_and_dest(get_extern_while_body(cop), start, end, dest_list, source_list, [],
                                             ifs_to_dest)

                    if dest_list and source_list:

                        # modify_conds(label1, conds_dict, cop, ast)

                        assign = get_label_assign_num(cop, labelname)

                        # print get_extern_while_body(cop)

                        if assign <= 2:
                            check_if_gen = CheckIfGenerator(start, end)
                            # print TreeGenerator().visit(get_extern_while_body(cop))
                            check_if_gen.visit(get_extern_while_body(cop))
                            # print check_if_gen.is_jumping
                            # print check_if_gen.is_blocking
                            # print "\n\nUNUL\n\n"
                            # print ifs_to_dest
                            if not check_if_gen.is_blocking and not check_if_gen.true_jump:
                                new_conds = add_ghost_assign_in_tree(cop, ifs_to_dest, label1)
                                trees_list.append(cop)
                            else:
                                # if labels != labels_sorted:
                                is_job = True
                                aux, is_job_aux = find_all_paths_between_two_nodes(cop, start, end)
                                test = take_2assigns_to_label_only(aux, labelname)

                                if test:
                                    trees_paths_list.append(test)





                        else:

                            aux, is_job_aux = find_all_paths_between_two_nodes(cop, start, end)
                            test = take_2assigns_to_label_only(aux, labelname)

                            if test:
                                if is_job_aux:
                                    is_job = True
                                trees_paths_list.append(test)

                # aici bag conditiile
            if new_conds:
                conds_dict[label2] = new_conds

        trees_dict[label1] = trees_list
        trees_paths_dict[label1] = trees_paths_list
        # break
    # print conds_dict.keys()
    return trees_dict, trees_paths_dict, is_job


def modify_cond(cond, new_vals):
    # print new_vals
    strings = ['pid', 'old', 'timeout']
    if isinstance(cond, UnaryOp):
        modify_cond(cond.expr, new_vals)
    elif isinstance(cond, ID):
        if not any(x in cond.name for x in strings):
            for val in new_vals:
                if cond.name in val:
                    cond.name = val
                    # new_vals.remove(val)

    elif isinstance(cond, StructRef):
        if isinstance(cond.name, ID):
            if not any(x in cond.name.name for x in strings):
                for val in new_vals:
                    if cond.name.name in val:
                        cond.name.name = val

    elif isinstance(cond.left, StructRef):
        if isinstance(cond.left.name, ID):
            if not any(x in cond.left.name.name for x in strings):
                for val in new_vals:
                    if cond.left.name.name in val:
                        cond.left.name.name = val
        if isinstance(cond.left.name, ArrayRef):
            array = cond.left.name
            if isinstance(array, ArrayRef):
                if isinstance(array.name, ID):
                    if not any(x in array.name.name for x in strings):
                        for val in new_vals:
                            if array.name.name in val:
                                cond.left.name.name.name = val

                else:
                    if not any(x in array.name.name.name for x in strings):
                        for val in new_vals:
                            if array.name.name.name in val:
                                cond.left.name.name.name.name = val
            else:
                print type(array)
                if not any(x in array.name.name.name for x in strings):
                    aux = array.name.name.name
                    for val in new_vals:
                        if aux in val:
                            cond.left.name.name.name.name = val

    elif isinstance(cond.left, ID):
        if not any(x in cond.left.name for x in strings):
            for val in new_vals:
                if cond.left.name in val:
                    cond.left.name = val

    elif isinstance(cond, BinaryOp):
        if isinstance(cond.left, BinaryOp):
            modify_cond(cond.left, new_vals)
        if isinstance(cond.right, BinaryOp):
            modify_cond(cond.right, new_vals)


def take_cond_name(cond, lista):
    """
    construieste lista cu variabile care sunt de adaugat
    primeste label pentru a stii in ce runda sunt si pentru a stii ce nume pun la variabila
    :param cond:
    :param label:
    :param lista:
    :return:
    """
    strings = ['pid', 'old', 'timeout']
    if isinstance(cond, UnaryOp):
        take_cond_name(cond.expr, lista)

    elif isinstance(cond, ID):
        if not any(x in cond.name for x in strings):
            aux = cond.name
            if aux not in lista:
                lista.append(aux)

    elif isinstance(cond, StructRef):
        if isinstance(cond.name, ID):
            if not any(x in cond.name.name for x in strings):
                aux = cond.name.name
                if aux not in lista:
                    lista.append(aux)

    elif isinstance(cond.left, StructRef):
        if isinstance(cond.left.name, ID):
            if not any(x in cond.left.name.name for x in strings):
                aux = cond.left.name.name
                if aux not in lista:
                    lista.append(aux)

        if isinstance(cond.left.name, ArrayRef):
            array = cond.left.name
            if isinstance(array, ArrayRef):
                if isinstance(array.name, ID):
                    if not any(x in array.name.name for x in strings):
                        aux = array.name.name
                        if aux not in lista:
                            lista.append(aux)

                else:
                    if not any(x in array.name.name.name for x in strings):
                        aux = array.name.name.name
                        if aux not in lista:
                            lista.append(aux)
            else:
                print type(array)
                if not any(x in array.name.name.name for x in strings):
                    aux = array.name.name.name
                    if aux not in lista:
                        lista.append(aux)

    elif isinstance(cond.left, ID):
        if not any(x in cond.left.name for x in strings):
            aux = cond.left.name
            if aux not in lista:
                lista.append(aux)

    elif isinstance(cond, BinaryOp):
        if isinstance(cond.left, BinaryOp):
            take_cond_name(cond.left, lista)
        if isinstance(cond.right, BinaryOp):
            take_cond_name(cond.right, lista)


def create_new_cond_name(cond_name, label, count):
    new_name = label + "_" + str(count) + "_" + cond_name

    return new_name


def create_new_assign(old_cond, new_cond, coord):
    global coord_aux

    coord_aux -= 1
    # coord = old_cond.coord
    new_coord = Coord(coord.file, coord.line, coord.column)
    new_coord.line = coord_aux
    new_coord.column = coord_aux

    assign = Assignment('=', ID(new_cond), ID(old_cond), new_coord)
    return assign


def add_ghost_assign_in_tree(tree, ifs_to_des, label):
    global added_vars
    """
    adauga in copac assignmentul de variabila si intoarce o lista de tupluri de genul (if de modificat, noile nume de variabile)
    :param tree:
    :param ifs_to_des:
    :param label:
    :return:
    """

    ifs_new_names = []
    for elem in ifs_to_des:
        conds_list = []
        new_conds_list = []
        take_cond_name(elem.cond, conds_list)
        # print conds_list
        for cond in conds_list:
            parent = find_parent(tree, elem)
            index = parent.block_items.index(elem)
            count = 0
            new_cond = create_new_cond_name(cond, label, count)

            # new_cond = correct_added_vars(cond, new_cond ,label,count)
            added_vars.append(new_cond)
            new_conds_list.append(new_cond)

            assign = create_new_assign(cond, new_cond, elem.coord)
            parent.block_items.insert(index, assign)

        # print new_conds_list
        aux = (elem, new_conds_list)
        ifs_new_names.append(aux)
        # print new_names
    return ifs_new_names


def correct_added_vars(cond, new_cond, label, count):
    i = 0
    while i < len(added_vars):
        if new_cond in added_vars:
            count += 1

        i += 1

    aux = create_new_cond_name(cond, label, count)
    return aux


def add_assign_in_tree(tree, label, variabile_old, original_ast):
    to_add = []

    if tree is not None:  # nu am idee unde ajunge aici pe None
        for i, item in enumerate(tree.block_items):
            if isinstance(item, For):
                add_assign_in_tree(item.stmt, label, variabile_old, original_ast)
            if isinstance(item, If):
                node = find_parent(original_ast, item)
                index = node.block_items.index(item)

                lista = []
                count = 0
                lab = "old_" + label.replace("ROUND", "") + str(count) + "_"
                new_lab = lab
                take_cond_name(node.block_items[index].cond, lab, lista)
                for element in lista:
                    if element in variabile_old:
                        count += 1
                        new_lab = "old_" + label.replace("ROUND", "") + str(count) + "_"
                        new_element = element.replace(lab, new_lab)

                        assign = Assignment("=", ID(new_element), ID(element), item.coord)
                    else:

                        assign = Assignment("=", ID(element), ID(element.replace(lab, "")), item.coord)
                    if index == 0:
                        modify_cond(node.block_items[index].cond, new_lab)
                        node.block_items.insert(index, assign)
                        index += 1
                        variabile_old.append(element)
                    elif isinstance(node.block_items[index - 1], Assignment):
                        if "mbox" in node.block_items[index - 1].lvalue.name and "old" not in node.block_items[
                            index - 1].lvalue.name:
                            modify_cond(node.block_items[index].cond, new_lab)
                            node.block_items.insert(index, assign)
                            index += 1
                            variabile_old.append(element)

                        elif "mbox" not in node.block_items[index - 1].lvalue.name:
                            modify_cond(node.block_items[index].cond, new_lab)
                            node.block_items.insert(index, assign)
                            index += 1
                            variabile_old.append(element)
                    elif not isinstance(node.block_items[index - 1], Assignment):
                        modify_cond(node.block_items[index].cond, new_lab)
                        node.block_items.insert(index, assign)
                        index += 1
                        variabile_old.append(element)

                # print type(tree.block_items[i].iftrue)
                add_assign_in_tree(tree.block_items[i].iftrue, label, variabile_old, original_ast)
                if tree.block_items[i].iffalse is not None:
                    add_assign_in_tree(tree.block_items[i].iffalse, label, variabile_old, original_ast)

    for index, element in to_add:
        tree.block_items.insert(index, element)


def add_assign_in_path(path, to_add):
    for i in xrange(len(path)):
        for k in to_add:
            if k[0] in path[i]:
                index = path[i].index(k[0])
                path[i].insert(index, k[1])


def add_ghost_assign(trees_dict, labels, original_ast):
    for x in labels:
        trees_list = trees_dict[x]
        for i in xrange(len(trees_list)):
            add_assign_in_tree(get_extern_while_body(trees_list[i]), x, [], original_ast)


def get_code_from_trees_only(trees_dict, labels):
    code = {}
    gen = TreeGenerator()
    for x in labels:
        code_for_label = []
        trees_list = trees_dict[x]
        # print x
        for tree in trees_list:
            code_for_label.append(gen.visit(get_extern_while_body(tree)))
            print gen.visit(get_extern_while_body(tree))

        code[x] = code_for_label
    return code


def take_2assigns_to_label_only(paths, labelname):
    """
    removes the paths which contain more than 2 assigns to label
    :param paths:paths list
    :param labelname: the name of the label
    :return:
    """
    aux = []
    for tuple in paths:
        tree = tuple[0]
        path = tuple[1]
        assign = 0
        for node in path:
            if isinstance(node, Assignment) and node.lvalue.name == labelname:
                assign += 1

        if assign > 2:
            aux.append(tuple)

    for x in aux:
        paths.remove(x)
    return paths


def print_code_from_trees_paths(trees_paths_dict, labels):
    for x in labels:
        if x in trees_paths_dict:
            paths = trees_paths_dict[x]
            for path in paths:
                if path is not None:
                    generate_c_code_from_paths_and_trees(path)


def print_code_from_trees(trees_dict, labels):
    gen = TreeGenerator()
    for x in labels:
        if x in trees_dict:
            trees_list = trees_dict[x]
            for tree in trees_list:
                print gen.visit(get_extern_while_body(tree))


def print_code(trees_dict, trees_paths_dict, labels):
    print_code_from_trees(trees_dict, labels)
    print_code_from_trees_paths(trees_paths_dict, labels)


def print_rounds(labels, trees_dict, trees_paths_dict, labelname, is_job):
    for label in labels[:len(labels) - 1]:
        if label == "AUX_ROUND":
            continue

        print "def round " + label + ":"

        print "  SEND():\n"

        if is_job:
            print "if(round == {0})".format(label)
            print "{"

        found_send_list = []
        history_of_strings = []

        for tree in trees_dict[label]:
            # print TreeGenerator().visit(get_extern_while_body(tree))
            gen = RoundGenerator("send", labelname, label)
            result = gen.visit(get_extern_while_body(tree))
            result = os.linesep.join([s for s in result.splitlines() if s])
            if gen.send_reached:
                if result not in history_of_strings:
                    print result
                    history_of_strings.append(result)
                found_send_list.append(True)
            else:
                found_send_list.append(False)

        list_of_lists_of_tuples = trees_paths_dict[label]
        for list_of_tuples in list_of_lists_of_tuples:
            for tuple_el in list_of_tuples:
                # print TreeGenerator().visit(get_extern_while_body(tuple_el[0]))
                gen = RoundGenerator("send", labelname, label, tuple_el[1])
                result = gen.visit(get_extern_while_body(tuple_el[0]))
                result = os.linesep.join([s for s in result.splitlines() if s])
                if gen.send_reached:
                    if result not in history_of_strings:
                        print result
                        history_of_strings.append(result)
                    found_send_list.append(True)
                else:
                    found_send_list.append(False)

        if is_job:
            print "}\n"

        print "  UPDATE():\n"

        if is_job:
            print "if(round == {0})".format(label)
            print "{"

        history_of_strings = []
        for i, tree in enumerate(trees_dict[label]):
            # print TreeGenerator().visit(get_extern_while_body(tree))
            gen = RoundGenerator("update", labelname, label)
            if not found_send_list[i]:
                gen.send_reached = True
            result = gen.visit(get_extern_while_body(tree))
            result = os.linesep.join([s for s in result.splitlines() if s])
            if result not in history_of_strings:
                print result
                history_of_strings.append(result)

        i = len(trees_dict[label])
        for list_of_tuples in list_of_lists_of_tuples:
            for tuple_el in list_of_tuples:
                # print TreeGenerator().visit(get_extern_while_body(tuple_el[0]))
                gen = RoundGenerator("update", labelname, label, tuple_el[1])
                if not found_send_list[i]:
                    gen.send_reached = True
                result = gen.visit(get_extern_while_body(tuple_el[0]))
                result = os.linesep.join([s for s in result.splitlines() if s])
                if result not in history_of_strings:
                    print result
                    history_of_strings.append(result)
                i = i + 1

        if is_job:
            print "}"
        print "\n"


def identify_epoch_jumps(ast_tree, epoch_name):
    epoch_list = get_epochs_assigns(ast_tree, epoch_name)
    if epoch_list is []:
        return False
    else:
        return True


def assign_unique_coord(elem, coord):
    global coord_aux
    coord_aux -= 1
    coord.column = coord_aux
    coord.line = coord_aux
    elem.coord = coord
    # return elem


def create_blockb(ast, epoch_name):
    ast_tree = duplicate_element(ast)
    if identify_epoch_jumps(ast_tree, epoch_name):
        epoch_jumps = get_epochs_assigns(get_extern_while_body(ast_tree), epoch_name)
        coord = get_extern_while_body(ast_tree).coord
        if len(epoch_jumps) == 1:
            epoch = epoch_jumps[0]

            parent_compound = find_parent(ast_tree, epoch)

            parent = find_parentAndrei(ast_tree, epoch)
            grandparent = find_parentAndrei(ast_tree, parent)

            index = grandparent.block_items.index(parent)

            to_add = grandparent.block_items[index + 1:]

            parent_compound.block_items.remove(epoch)

            assign = create_new_assign('true', 'b', coord)  # create assign b = true
            assign_unique_coord(assign, coord)

            length = len(parent_compound.block_items)
            parent_compound.block_items.insert(length-1, assign) #inserez elementul, dar nu pe ultima pozitie
            parent_compound.block_items = parent_compound.block_items + to_add
            # parent_compound.block_items.append(assign)
            block_b = parent_compound

            return block_b

    return ast_tree


def create_block_a(ast, epoch_name):
    ast_tree = duplicate_element(ast)
    if identify_epoch_jumps(ast_tree, epoch_name):
        epoch_jumps = get_epochs_assigns(get_extern_while_body(ast_tree), epoch_name)
        coord = get_extern_while_body(ast_tree).coord
        if len(epoch_jumps) == 1:
            epoch = epoch_jumps[0]

            parent_compound = find_parent(ast_tree, epoch)
            index = parent_compound.block_items.index(epoch)

            assign_false = create_new_assign('false', 'b', coord)  # create assign b = false
            assign_unique_coord(assign_false, coord)

            comp_false = Compound([assign_false], coord)
            assign_unique_coord(comp_false, coord)

            lista = parent_compound.block_items[index + 1:]
            new_comp = Compound(lista, coord)
            assign_unique_coord(new_comp, coord)

            cond = BinaryOp('==', ID('iter'), ID(epoch_name))
            iter_if = If(cond, new_comp, comp_false)
            assign_unique_coord(iter_if, coord)

            parent_compound.block_items = parent_compound.block_items[:index + 1]
            parent_compound.block_items.append(iter_if)

            # print generator.visit(get_extern_while_body(ast_tree))
            return get_extern_while_body(ast_tree)


def check_epoch_jumps(ast_tree, epoch_name):
    cop = duplicate_element(ast_tree)
    cop_aux = duplicate_element(ast_tree)
    if identify_epoch_jumps(cop, epoch_name):
        epoch_jumps = get_epochs_assigns(get_extern_while_body(cop), epoch_name)
        coord = get_extern_while_body(cop).coord
        if len(epoch_jumps) == 1:
            epoch = epoch_jumps[0]

            parent_compound = find_parent(cop_aux, epoch)

            block_a = create_block_a(cop, 'epoch')
            assign_unique_coord(block_a, coord)

            block_b = create_blockb(cop, epoch_name)
            assign_unique_coord(block_b, coord)
            # print generator.visit(block_a)
            # print generator.visit(block_b)

            iter_assign = create_new_assign('0', 'iter', coord)  # initialize iter to 0

            iter_inc = UnaryOp('p++', ID('iter'), coord)  # iter ++
            assign_unique_coord(iter_inc, coord)

            assign_true = create_new_assign('true', 'b', coord)  # create assign b = true
            assign_unique_coord(assign_true, coord)

            # assign_false = create_new_assign('false', 'b', coord)  # create assign b = false
            # assign_unique_coord(assign_false, coord)

            main = get_main_function(cop).body
            main.block_items.insert(0, assign_true)
            main.block_items.insert(0, iter_assign)
            # am adaugat in arbore initializarea lui b si a lui iter

            cond = BinaryOp('==', ID('iter'), ID(epoch_name))
            uncond = UnaryOp('!', ID('b'))

            iter_if = If(cond, None, None)
            assign_unique_coord(iter_if, coord)

            blockb_if = If(uncond, block_b, block_a)
            comp_block_b_if = Compound([blockb_if], coord)
            assign_unique_coord(comp_block_b_if, coord)

            iter_if.iftrue = comp_block_b_if
            body = get_extern_while_body(cop)
            body.block_items.insert(1, iter_if)
            body.block_items.insert(1, iter_inc)
            body.block_items = body.block_items[:3]

            return cop


def async_to_async(ast, epoch_name, filename):
    if identify_epoch_jumps(ast, epoch_name):
        async = check_epoch_jumps(ast, epoch_name)
        # print generator.visit(async)
        with open('aux.c', 'w') as file:
            file.write(generator.visit(async))
        aux = parse_file(filename="aux.c", use_cpp=False)
        return aux
    else:
        return ast


def take_code_from_file(ast, filename, labelname):
    x = copy.deepcopy(ast)
    labels_sorted = get_labels_order(filename, labelname)
    labels = get_labels(filename, labelname)

    ast = async_to_async(ast, 'epoch', filename)
    # print generator.visit(ast)
    trees_dict, trees_paths_dict, is_job = get_paths_trees(ast, labels, labels_sorted, labelname)

    # print_code(trees_dict, trees_paths_dict, labels_sorted)

    print_rounds(labels_sorted, trees_dict, trees_paths_dict, labelname, is_job)

    return trees_dict
