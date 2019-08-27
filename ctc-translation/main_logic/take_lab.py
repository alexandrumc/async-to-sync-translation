"""
main_logic: take_lab.py

This module contains multiple wrappers which rewrite the asynchronous code to
synchronous. It also contains functions which determine details about algorithms
(like the list of parameters) and the function which extracts sequences between
two round assignments
"""

import copy
import os
import sys
from cStringIO import StringIO

from pycparser import c_generator, parse_file
from pycparser.plyparser import Coord
from pycparser.c_ast import While, Assignment, ID, If, FuncDef, FileAST, UnaryOp, BinaryOp, StructRef, ArrayRef, \
    For, Compound, Continue, FuncCall, FuncDecl, IdentifierType, Decl, TypeDecl

from utils.utils import get_label, duplicate_element, get_label_assign_num, generate_c_code_from_paths_and_trees, \
    find_parent, find_node, get_epochs_assigns, find_parentUpdated, get_main_function, find_lca, get_recv_whiles, \
    get_extern_while_body, is_upon_syntax, get_init_section, get_extern_while
from utils.generators import TreeGenerator, RoundGenerator, CheckIfGenerator, WhileAlgoVisitor, DeclAlgoVisitor, \
    AllVarsAlgoVisitor, AssigDummyVisitor, SendLoopsVisitor, SwitchToOldVars, JumpPhaseVisitor

from compute_paths import find_all_paths_between_two_nodes, prune_tree

from modify_whiles import coord_aux, whiles_to_if, identify_recv_exits
from mbox_removal import remove_mbox


generator = c_generator.CGenerator()

added_vars = []

var_b = 0


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
            # print ext
            amain_body = ext
            if amain_body is not None:
                for operation in amain_body.body:
                    if isinstance(operation, While):
                        return operation.stmt


def get_labels(filename, labelname):
    """
    appends the label value in the list when it is found, if it is already in the list, it is ignored
    :param filename:
    :param labelname:
    :return: list with labels values
    """
    # print labelname
    labels = []
    lab = labelname + "="
    with open(filename) as f:
        lines = f.readlines()
    for each in lines:
        if labelname in each:
            aux = each.replace(" ", "")
            # print aux
            aux = aux[aux.index(labelname):]
            # print aux
            if "\n" in aux:
                aux = aux.replace("\n", "")
            aux = aux.replace(lab, "")
            aux = aux.replace(";", "")
            # aux = aux.replace(" ","")
            if not aux[-1].isalnum():  # no idea why this happens :O
                aux = aux[:len(aux) - 1]
            if aux.endswith("ROUND"):
                # print aux, " se termina cu"
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
                if elem.iftrue:
                    if isinstance(elem.iftrue.block_items[0], If):
                        context.append(elem)
                        get_context(elem.iftrue, context)
                    if isinstance(elem.iftrue.block_items[0], Assignment):
                        context.append(elem)
                        break
                if elem.iffalse:
                    if isinstance(elem.iffalse.block_items[0], If):
                        context.append(elem)
                        get_context(elem.iffalse, context)
                    if isinstance(elem.iffalse.block_items[0], Assignment):
                        context.append(elem)
                        break
                else:
                    break

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


def add_old_vars_filter(msg_field, vars_list, trees_dict, trees_paths_dict):
    """
    This filter will apply a visitor to every raw input extracted, only for the UPON syntax. At the end
    of this filter, old vars should be applied where they are necessary
    :param trees_dict:
    :param trees_paths_dict:
    :param msg_field:
    :param vars_list:
    :return:
    """

    for key, value in trees_dict.iteritems():
        round_name = key
        round_label = msg_field["round_field"]

        for el in value:
            v = SwitchToOldVars(vars_list, round_label, round_name)
            v.visit(get_extern_while_body(el))

            for x in v.other_assigs:
                p = find_parent(get_main_function(el), x)

                if not isinstance(p, Compound):
                    continue

                ind = p.block_items.index(x)
                for y in vars_list:
                    id1 = ID("old_" + y, x.coord)
                    id2 = ID(y, x.coord)

                    assig = Assignment("=", id1, id2, x.coord)
                    p.block_items.insert(ind, assig)

    """
    for key, value in trees_paths_dict.iteritems():
        round_name = key
        round_label = msg_field["round_field"]

        for el in value:
            v = SwitchToOldVars(vars_list, round_label, round_name)
            v.visit(get_extern_while_body(el))

            for x in v.other_assigs:
                p = find_parent(el, x)

                if not isinstance(p, Compound):
                    continue

                ind = p.block_items.index(x)
                for y in vars_list:
                    id1 = ID("old_" + y, x.coord)
                    id2 = ID(y, x.coord)

                    assig = Assignment("=", id1, id2, x.coord)
                    p.block_items.insert(ind, assig)
    """


def get_paths_trees(ast, labels, labels_sorted, labelname, is_upon):
    aux_dict = {}
    trees_dict = {}
    trees_paths_dict = {}
    used_old_vars = []
    added_ifs_assignment = []
    is_job = False
    labels_sorted.reverse()
    for label1 in labels_sorted:

        if label1 == "AUX_ROUND" or label1 == "ERR_ROUND":
            continue
        trees_list = []
        trees_paths_list = []
        labels_start = get_label(ast, labelname, label1)

        for label2 in labels_sorted:
            if label2 != label1:
                labels_end = get_label(ast, labelname, label2)
                # print label1, label2

                for start in labels_start:
                    # la final sa bag conditiile pt urmatorul element de iterat
                    for end in labels_end:
                        cop = duplicate_element(ast)
                        # prune_tree(get_extern_while_body_from_func(cop),start,end,[],[])

                        dest_list = []
                        source_list = []
                        prune_tree(get_extern_while_body(cop), start, end, dest_list, source_list)

                        #print "Print for round " + label1
                        #print generator.visit(get_extern_while_body(cop))
                        #print "Done printing one sequence \n\n"
                        #remove_null_if(get_extern_while_body(cop))

                        if dest_list and source_list:
                            context = []
                            if not is_upon:
                                get_context(get_extern_while_body(cop), context)
                            add_ghost_assign_in_tree(ast, context, used_old_vars, added_ifs_assignment, aux_dict)
                            for elem in aux_dict:
                                test = find_node(cop, elem)
                                # print generator.visit(elem.cond), aux_dict[elem]
                                if test and test in context:
                                    # print test
                                    # print label1, label2
                                    # print generator.visit(test.cond), "before"
                                    modify_cond(test.cond, aux_dict[elem])
                                    # print generator.visit(test.cond), "after"
                                # print generator.visit(elem.cond)
                                # print generator.visit(cop), "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                                # break
                            # print generator.visit(ast)
                            # for elem in context:
                            #     print generator.visit(elem.cond), elem.coord.line
                            #     list = []
                            #     take_cond_name(elem.cond,list)
                            #     print list

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
                                    # new_conds = add_ghost_assign_in_tree(cop, ifs_to_dest, label1)
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

                    # conditions are added here
        trees_dict[label1] = trees_list
        trees_paths_dict[label1] = trees_paths_list
        if label1 == "FIFTH_ROUND" or label1 == "SIXTH_ROUND":
            print trees_dict[label1]
            print trees_paths_dict[label1]

        # break
    # print conds_dict.keys()
    # print generator.visit(ast)
    remove_dummy_assigs(trees_dict, trees_paths_dict)
    return trees_dict, trees_paths_dict, is_job


def remove_dummy_assigs(trees_dict, trees_paths_dict):
    """
    Because in order to parse out_internal -> return_from_inner cases we needed to
    use some dummy round = ERR_ROUND assigs, now we need to delete them as those are
    considered dead_code
    :return:
    """
    v = AssigDummyVisitor()
    for td in trees_dict.values():
        for aux_td in td:
            v.result = []
            aux_td = get_extern_while_body(aux_td)
            v.visit(aux_td)

            for el in v.result:
                p = find_parent(aux_td, el)
                if isinstance(p, Compound):
                    ind = p.block_items.index(el)
                    if ind >= 1 and isinstance(p.block_items[ind - 1], FuncCall):
                        if p.block_items[ind - 1].name.name == "return_from_inner":
                            del p.block_items[ind]

    # Probably for trees_path_dict should be done as well, but crashes
    """
    for td in trees_paths_dict.values():
        for aux_td in td:
            v.result = []
            aux_td = get_extern_while_body(aux_td)
            v.visit(aux_td)

            for el in v.result:
                p = find_parent(aux_td, el)
                if isinstance(p, Compound):
                    ind = p.block_items.index(el)
                    if ind >= 1 and isinstance(p.block_items[ind - 1], FuncCall):
                        if p.block_items[ind - 1].name.name == "return_from_inner":
                            del p.block_items[ind]

    return
    """


def modify_cond(cond, new_vals):
    # print "AAAAAAAAAAAAA"
    strings = ['pid', 'old', 'timeout', 'id', 'myid']

    # print generator.visit(cond), type(cond)

    if isinstance(cond, UnaryOp):
        modify_cond(cond.expr, new_vals)

    elif isinstance(cond, ID):
        if not any(x in cond.name for x in strings):
            for val in new_vals:
                if cond.name in val:
                    cond.name = val

    # elif isinstance(cond.right, ID) and "leader" in cond.right.name:
    #     for val in new_vals:
    #         if cond.right.name in val:
    #             cond.right.name = val

    # new_vals.remove(val)
    elif isinstance(cond, ArrayRef):
        if isinstance(cond.name, ID):
            if not any(x in cond.name.name for x in strings):
                for val in new_vals:
                    if cond.name.name in val:
                        cond.name.name = val
    elif isinstance(cond, StructRef):
        modify_cond(cond.name, new_vals)
        # if isinstance(cond.name, StructRef):
        #     if isinstance(cond.name.name, ID):
        #         print cond.name.name.name
        if isinstance(cond.name, ID):
            if not any(x in cond.name.name for x in strings):
                for val in new_vals:
                    if cond.name.name in val:
                        cond.name.name = val

    elif isinstance(cond, BinaryOp) and isinstance(cond.left, StructRef):
        modify_cond(cond.left, new_vals)
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
                # print type(array)
                if not any(x in array.name.name.name for x in strings):
                    aux = array.name.name.name
                    for val in new_vals:
                        if aux in val:
                            cond.left.name.name.name.name = val

    elif isinstance(cond, BinaryOp) and isinstance(cond.left, ID):
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
    strings = ['pid', 'old', 'timeout', 'id', 'myid', 'iter']

    # print generator.visit(cond), generator.visit(cond.right), type(cond.right)
    # print generator.visit(cond)
    if isinstance(cond, FuncCall):
        pass


    elif isinstance(cond, UnaryOp):
        print cond.coord
        take_cond_name(cond.expr, lista)


    elif isinstance(cond, ID):
        if not any(x in cond.name for x in strings):
            aux = cond.name
            if aux not in lista:
                lista.append(aux)

    # elif isinstance(cond.right, ID) and cond.right.name == 'leader':
    #     aux = cond.right.name
    #     if aux not in lista:
    #         lista.append(aux)

    elif isinstance(cond, StructRef):
        # print generator.visit(cond)
        if isinstance(cond.name, ID):
            if not any(x in cond.name.name for x in strings):
                aux = cond.name.name
                if aux not in lista:
                    lista.append(aux)

    elif isinstance(cond.left, StructRef):
        # print generator.visit(cond), type(cond.left)
        take_cond_name(cond.left, lista)
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
                # print type(array)
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


def create_new_cond_name(cond_name, count):
    new_name = 'old' + "_" + str(count) + "_" + cond_name

    return new_name


def create_new_assign(old_cond, new_cond, coord):
    global coord_aux

    coord_aux -= 1
    # coord = old_cond.coord
    new_coord = Coord(coord.file, coord.line, coord.column)
    new_coord.line = coord_aux
    new_coord.column = coord_aux
    # print type(new_cond), type(new_cond)
    assign = Assignment('=', ID(new_cond), ID(old_cond), new_coord)
    return assign


def add_ghost_assign_in_tree(tree, context, used_old_vars, added_ifs_assignment, aux_dict):
    global added_vars
    """
    adauga in copac assignmentul de variabila si intoarce o lista de tupluri de genul (if de modificat, noile nume de variabile)
    :param tree:
    :param ifs_to_des:
    :param label:
    :return:
    """

    for elem in context:
        conds_list = []
        new_conds_list = []
        take_cond_name(elem.cond, conds_list)

        for cond in conds_list:
            # print "aaaaaaaaa", cond
            parent = find_parent(tree, elem)
            index = parent.block_items.index(elem)
            count = used_old_vars.count(cond)
            if elem not in added_ifs_assignment:
                used_old_vars.append(cond)

                new_cond = create_new_cond_name(cond, count)
                new_conds_list.append(new_cond)
                assign = create_new_assign(cond, new_cond, elem.coord)
                # print generator.visit(assign)
                if assign:
                    parent.block_items.insert(index, assign)

                #
        #         added_vars.append(new_cond)
        #         new_conds_list.append(new_cond)
        #
        # assign = create_new_assign(cond, new_cond, elem.coord)
        #
        #
        #     aux = (elem, new_conds_list)
        #     ifs_new_names.append(aux)
        #     # print new_names
        added_ifs_assignment.append(elem)
        if new_conds_list:
            aux_dict[elem] = new_conds_list

    # return ifs_new_names


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


def print_rounds(labels, trees_dict, trees_paths_dict, labelname, is_job, delete_round_phase, message, variables, is_upon):
    labels.reverse()
    # print labels[:len(labels) - 1]
    save_round = ""
    for label in labels[:len(labels) - 1]:
        if label == "AUX_ROUND":
            continue

        print "def round " + label + ":"

        print "  SEND():\n"

        res = ""
        res_aux = ""

        if is_job:
            res += "if(round == {0})".format(label)
            res += "{"
            res += "\n"

        found_send_list = []
        history_of_strings = []



        for tree in trees_dict[label]:
            # print TreeGenerator().visit(get_extern_while_body(tree))
            gen = RoundGenerator("send", labelname, label, delete_round_phase, message, variables, save_round, is_upon)


            result = gen.visit(get_extern_while_body(tree))
            if gen.remember_round != "":
                save_round = gen.remember_round
            result = os.linesep.join([s for s in result.splitlines() if s])
            if gen.send_reached:
                if result not in history_of_strings:
                    res_aux += result
                    res_aux += "\n"
                    history_of_strings.append(result)
                found_send_list.append(True)
            else:
                found_send_list.append(False)

        list_of_lists_of_tuples = trees_paths_dict[label]
        for list_of_tuples in list_of_lists_of_tuples:
            for tuple_el in list_of_tuples:
                # print TreeGenerator().visit(get_extern_while_body(tuple_el[0]))
                gen = RoundGenerator("send", labelname, label, delete_round_phase, message, variables, save_round,
                                     is_upon, tuple_el[1])
                result = gen.visit(get_extern_while_body(tuple_el[0]))
                if gen.remember_round != "":
                    save_round = gen.remember_round
                result = os.linesep.join([s for s in result.splitlines() if s])
                if gen.send_reached:
                    if result not in history_of_strings:
                        res_aux += result
                        res_aux += "\n"
                        history_of_strings.append(result)
                    found_send_list.append(True)
                else:
                    found_send_list.append(False)

        res_aux_copy = copy.copy(res_aux)
        res_aux_copy = res_aux_copy.replace(" ", "")
        if res_aux_copy != "":
            res += res_aux
            if is_job:
                res += "}\n"
            print res

        print "  UPDATE():\n"

        if is_job:
            print "if(round == {0})".format(label)
            print "{"

        history_of_strings = []
        for i, tree in enumerate(trees_dict[label]):
            # print TreeGenerator().visit(get_extern_while_body(tree))
            gen = RoundGenerator("update", labelname, label, delete_round_phase, message, variables, save_round, is_upon)
            if not found_send_list[i]:
                gen.send_reached = True
            result = gen.visit(get_extern_while_body(tree))
            if gen.remember_round != "":
                save_round = gen.remember_round
            result = os.linesep.join([s for s in result.splitlines() if s])
            if result not in history_of_strings:
                print result
                history_of_strings.append(result)

        i = len(trees_dict[label])
        for list_of_tuples in list_of_lists_of_tuples:
            for tuple_el in list_of_tuples:
                # print TreeGenerator().visit(get_extern_while_body(tuple_el[0]))
                gen = RoundGenerator("update", labelname, label, delete_round_phase, message, variables, save_round,
                                     is_upon, tuple_el[1])
                if not found_send_list[i]:
                    gen.send_reached = True
                result = gen.visit(get_extern_while_body(tuple_el[0]))
                if gen.remember_round != "":
                    save_round = gen.remember_round
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
    # print epoch_list
    # for elem in epoch_list:
    #     print elem.coord
    if not epoch_list:
        # print "epoch list e []"
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


def take_code_between_epoch_assigns(ast_tree, epoch1, epoch2):
    lca = find_lca(ast_tree, epoch1, epoch2)

    epoch1_parent = find_parentUpdated(ast_tree, epoch1)
    epoch2_parent = find_parentUpdated(ast_tree, epoch2)

    epoch1_grandparent = find_parentUpdated(ast_tree, epoch1_parent)
    epoch2_grandparent = find_parentUpdated(ast_tree, epoch2_parent)

    while lca != epoch1_grandparent:
        epoch1_parent = epoch1_grandparent
        epoch1_grandparent = find_parentUpdated(ast_tree, epoch1_parent)
    while lca != epoch2_grandparent:
        epoch2_parent = epoch2_grandparent
        epoch2_grandparent = find_parentUpdated(ast_tree, epoch2_parent)

    index1 = lca.block_items.index(epoch1_parent)
    index2 = lca.block_items.index(epoch2_parent)

    code_between_epochs = lca.block_items[index1 + 1:index2]

    return code_between_epochs


def insert_assigns(ast_tree, epoch_name, var_num):
    extern_while_body = get_extern_while_body(ast_tree)
    coord = extern_while_body.coord
    main = get_main_function(ast_tree).body

    iter_assign = create_new_assign('0', 'iter', coord)
    assign_unique_coord(iter_assign, coord)

    for i in xrange(1, var_num + 1):
        assign = create_new_assign('true', 'b' + str(i), coord)  # create assign b = true
        assign_unique_coord(assign, coord)
        main.block_items.insert(0, assign)

    main.block_items.insert(0, iter_assign)


def more_epoch_jumps(ast_tree, epoch_name):
    new_ast = duplicate_element(ast_tree)
    epoch_jumps = get_epochs_assigns(get_extern_while_body(ast_tree), epoch_name)
    epoch_jumps.reverse()
    coord = epoch_jumps[0].coord

    insert_assigns(new_ast, epoch_name, len(epoch_jumps))

    ifs_list = []
    for i in xrange(len(epoch_jumps)):
        block_b = identify_block_b(ast_tree, epoch_jumps[i], epoch_name)
        new_if = create_blockb_if(block_b, epoch_jumps[i])
        ifs_list.append(new_if)

    remaining_code = get_extern_while_body(ast_tree)

    cond = BinaryOp('==', ID('iter'), ID(epoch_name))
    iter_if = If(cond, None, None)
    assign_unique_coord(iter_if, coord)

    ifs_list[-1].iffalse = remaining_code
    for i in xrange(len(ifs_list)):
        if i + 1 in xrange(len(ifs_list)):
            comp = Compound([ifs_list[i + 1]])
            ifs_list[i].iffalse = comp

    aux = ifs_list[0]

    new_comp = Compound([aux])
    iter_if.iftrue = new_comp

    iter_inc = UnaryOp('p++', ID('iter'), coord)  # iter ++
    assign_unique_coord(iter_inc, coord)

    new_while_body = Compound([iter_inc, iter_if], coord)

    body = get_extern_while_body(new_ast)
    body.block_items = new_while_body

    # print generator.visit(new_ast)
    return new_ast


def create_blockb_if(block_b, epoch):
    """
    creates the if from the bB block
    :param block_b:
    :param epoch:
    :return:
    """
    global var_b

    new_cond = UnaryOp('!', ID('b' + str(var_b)))

    iffalse_comp = Compound([], epoch.coord)

    new_if = If(new_cond, block_b, iffalse_comp, epoch.coord)
    assign_unique_coord(new_if, epoch.coord)

    return new_if


def identify_block_b(ast, epoch, epoch_name):
    """
    identifies and creates the bB block
    :param ast:
    :param epoch:
    :param epoch_name:
    :return:
    """
    global var_b
    ast_tree = duplicate_element(ast)
    parent_comp = find_parent(ast_tree, epoch)
    parent = find_parentUpdated(ast_tree, epoch)
    # print generator.visit(parent_comp)
    epoch_index = parent_comp.block_items.index(epoch)
    to_add = parent_comp.block_items[epoch_index + 1:]

    changed_parent = True
    while changed_parent:
        grandparent_comp = find_parent(ast_tree, parent)
        grandparent = find_parentUpdated(ast_tree, parent)

        # print type(grandparent_comp.block_items)
        # print generator.visit(grandparent_comp)
        # print generator.visit(parent)
        index = grandparent_comp.block_items.index(parent)
        to_add = to_add + grandparent_comp.block_items[index + 1:]
        parent = grandparent
        if grandparent_comp == grandparent:
            changed_parent = False
        # print index

    block_b = Compound(to_add, epoch.coord)
    assign_unique_coord(block_b, epoch.coord)

    var_b += 1

    assign = create_new_assign('true', 'b' + str(var_b), epoch.coord)  # create assign b = true
    assign_unique_coord(assign, epoch.coord)
    block_b.block_items.insert(len(block_b.block_items), assign)
    modify_block(ast, epoch, epoch_name)

    return block_b


def modify_block(ast_tree, epoch, epoch_name):
    """
    modifies the block, assigns the bB variable properly
    :param ast_tree:
    :param epoch:
    :param epoch_name:
    :return:
    """
    parent_comp = find_parent(ast_tree, epoch)
    epoch_index = parent_comp.block_items.index(epoch)
    cond = BinaryOp('!=', ID('iter'), ID(epoch_name))

    new_if = If(cond, None, None, epoch.coord)
    assign_unique_coord(new_if, epoch.coord)

    assign_b_tofalse = create_new_assign('false', 'b' + str(var_b), epoch.coord)
    assign_unique_coord(assign_b_tofalse, epoch.coord)
    assign_b_totrue = create_new_assign('true', 'b' + str(var_b), epoch.coord)
    assign_unique_coord(assign_b_totrue, epoch.coord)

    cont = Continue(epoch.coord)

    if_true = Compound([assign_b_tofalse, cont], epoch.coord)
    assign_unique_coord(if_true, epoch.coord)

    if_false = Compound([assign_b_totrue], epoch.coord)
    assign_unique_coord(if_false, epoch.coord)

    new_if.iftrue = if_true
    new_if.iffalse = if_false

    parent_comp.block_items.insert(epoch_index + 1, new_if)


def async_to_async(ast, epoch_name):
    """
    writest the ast in an auxiliary file and reads it again(to keep the coords safe)
    :param ast:
    :param epoch_name:
    :return: the new ast or the original one if there is no epoch jump
    """
    if identify_epoch_jumps(ast, epoch_name):
        aux_ast = more_epoch_jumps(ast, epoch_name)
        # print generator.visit(async)
        with open('aux.c', 'w') as file:
            file.write(generator.visit(aux_ast))
        aux = parse_file(filename="aux.c", use_cpp=False)
        # print generator.visit(aux)
        # print "\n\nLaunched procedure for async to async rewriting(epoch jumps)\n\n"
        return aux
    else:
        return ast


def syntax_each_algo(ast_tree, rounds_list, msg_structure_fields):
    """
    Iterate through every while algo node and decide its syntax
    :return:
    """
    syntax_dict = {}
    v = WhileAlgoVisitor(rounds_list, msg_structure_fields)
    v.visit(ast_tree)

    for el in v.result_list:
        rank = WhileAlgoVisitor.get_rank_of_algo(el.stmt, rounds_list, msg_structure_fields)

        if rank in syntax_dict:
            continue

        syntax_dict[rank] = is_upon_syntax(el, msg_structure_fields, rounds_list)

    return syntax_dict


def check_phase_jump(node, rank, msg_structure_field):
    """
    Given a while algo node, check if there is any kind of phase jump in it
    :param node:
    :param rank:
    :param msg_structure_field:
    :return:
    """
    v = JumpPhaseVisitor(msg_structure_field[rank]["phase_field"])
    v.visit(node)

    return v.result, v.assig_list


def apply_ifs_for_roundjumps(trees_dict, trees_paths_dict, round_name):
    """
    If I detect round jumps, then every round has to execute only
    if round variable is equal to the current round value
    :param trees_dict:
    :param trees_paths_dict:
    :return:
    """
    # Insert round jump ifs extracting from FileAST
    for keys, list_asts in trees_dict.iteritems():
        for _ast_ in list_asts:
            while_node = get_extern_while(_ast_)
            my_coord = Coord(while_node.stmt.coord.file, -1, -1)

            # Create the if node
            id_iter = ID(round_name, my_coord)
            id_phase = ID(keys, my_coord)
            cond_node = BinaryOp("==", id_iter, id_phase, my_coord)
            if_node = If(cond_node, while_node.stmt, None, my_coord)
            while_node.stmt = Compound([if_node], while_node.coord)

    # Insert round jump for paths with multiple round assigs
    for keys, _list_ in trees_paths_dict.iteritems():
        for _tuple_list_ in _list_:
            for _tuple_ in _tuple_list_:
                my_coord = Coord(_tuple_[0].coord.file, -1, -1)
                # Create the if node
                id_iter = ID(round_name, my_coord)
                id_phase = ID(keys, my_coord)
                cond_node = BinaryOp("==", id_iter, id_phase, my_coord)
                if_node = If(cond_node, _tuple_[0], None, my_coord)
                comp = Compound([if_node], _tuple_[0].coord)
                aux_lst = list(_tuple_)
                aux_lst[0] = comp
                ind = _tuple_list_.index(_tuple_)
                _tuple_list_[ind] = tuple(aux_lst)


def apply_ifs_for_phasejumps(trees_dict, trees_paths_dict):
    """
    If I detect phase jumps, then every round has to execute only
    if iter variable is equal to PHASE variable
    :param trees_dict:
    :param trees_paths_dict:
    :return:
    """
    # Insert phase jump ifs extracting from FileAST
    for list_asts in trees_dict.values():
        for _ast_ in list_asts:
            while_node = get_extern_while(_ast_)
            my_coord = Coord(while_node.stmt.coord.file, -1, -1)

            # Create the if node
            id_iter = ID("iter", my_coord)
            id_phase = ID("PHASE", my_coord)
            cond_node = BinaryOp("==", id_iter, id_phase, my_coord)
            if_node = If(cond_node, while_node.stmt, None, my_coord)
            while_node.stmt = Compound([if_node], while_node.coord)

        # Insert phase jump for paths with multiple round assigs
        for keys, _list_ in trees_paths_dict.iteritems():
            for _tuple_list_ in _list_:
                for _tuple_ in _tuple_list_:
                    my_coord = Coord(_tuple_[0].coord.file, -1, -1)
                    # Create the if node
                    id_iter = ID("iter", my_coord)
                    id_phase = ID("PHASE", my_coord)
                    cond_node = BinaryOp("==", id_iter, id_phase, my_coord)
                    if_node = If(cond_node, _tuple_[0], None, my_coord)
                    comp = Compound([if_node], _tuple_[0].coord)
                    aux_lst = list(_tuple_)
                    aux_lst[0] = comp
                    ind = _tuple_list_.index(_tuple_)
                    _tuple_list_[ind] = tuple(aux_lst)


def isolate_jump_phase(trees_dict, trees_paths_dict, rank, msg_structure_fields):

    for tree_list in trees_dict.values():
        for tree in tree_list:
            v = JumpPhaseVisitor(msg_structure_fields[rank]["phase_field"])
            v.visit(get_extern_while(tree))

            # Change to name to PHASE for unary op and incr assig
            for el in v.incr_unary:
                el.expr.name = "PHASE"

            for el in v.incr_assig:
                el.lvalue.name = "PHASE"
                el.rvalue.left.name = "PHASE"

    for keys, _list_ in trees_paths_dict.iteritems():
        for _tuple_list_ in _list_:
            for _tuple_ in _tuple_list_:
                v = JumpPhaseVisitor(msg_structure_fields[rank]["phase_field"])
                v.visit(_tuple_[0])

                # Change to name to PHASE for unary op and incr assig
                for el in v.incr_unary:
                    el.expr.name = "PHASE"

                for el in v.incr_assig:
                    el.lvalue.name = "PHASE"
                    el.rvalue.left.name = "PHASE"

            # isolate_jump_phase_tree(v.assig_list, get_main_function(tree), offset, initial_checks)

    # Add initial ifs for each jump at the beginning of first round
    """
    tree_list = trees_dict[first_round_name]
    if len(tree_list) > 0:
        while_node = get_extern_while(tree_list[0])

        for el in initial_checks:
            el.iftrue.block_items[len(el.iftrue.block_items) - 1].rvalue.name = "True"

            while_node.stmt.block_items.insert(0, el)
    """


def isolate_jump_phase_tree(assig_list, start_node, offset, initial_checks):

    for assig in assig_list:
        if isinstance(assig.lvalue, ID):
            assig.lvalue.name = "PHASE"

        if isinstance(assig.rvalue, BinaryOp):
            if isinstance(assig.rvalue.left, ID):
                assig.rvalue.left.name = "PHASE"
        continue
        p = find_parent(start_node, assig)

        if not isinstance(p, Compound):
            continue

        ind = p.block_items.index(assig)

        nodes_list = []
        aux_ind = ind + 1
        while aux_ind < len(p.block_items):
            nodes_list.append(p.block_items[aux_ind])
            aux_ind += 1

        while ind < len(p.block_items) - 1:
            del p.block_items[ind+1]

        nodes_list.append(Assignment("=", ID("b" + str(offset+assig_list.index(assig)), assig.coord),
                                     ID("False", assig.coord), assig.coord))
        # Create the if node
        id_iter = ID("iter", assig.coord)
        id_phase = ID("PHASE", assig.coord)
        cond_node = BinaryOp("==", id_iter, id_phase, assig.coord)
        comp = Compound(nodes_list, assig.coord)
        comp_false = Compound([Assignment("=", ID("b" + str(offset+assig_list.index(assig)), assig.coord),
                                          ID("True", assig.coord), assig.coord)], assig.coord)
        if_node = If(cond_node, comp, comp_false, assig.coord)

        # Now, for the initial execution
        cond_node2 = UnaryOp('!', ID("b" + str(offset+assig_list.index(assig)), assig.coord), assig.coord)
        initial_checks.append(If(cond_node2, copy.deepcopy(comp), None, assig.coord))

        p.block_items.append(if_node)


def construct_if_cond_node(left_name, right_name, coord):
    """
    Given, two names, returns a node if (left_name == right_name)
    :param left_name:
    :param right_name:
    :param coord
    :return:
    """
    id_left = ID(left_name, coord)
    id_right = ID(right_name, coord)

    cond = BinaryOp("==", id_left, id_right, coord)

    if_node = If(cond, None, None, coord)

    return if_node


def update_code_nondet(while_algo, assig_list, outer_while):
    """
    After finding all jumps, add nondeterministic variables
    :param while_algo:
    :param assig_list:
    :return:
    """
    raise_warning = False
    my_coord = Coord(while_algo.stmt.coord.file, -1, -1)
    for assig in assig_list:
        p = find_parent(while_algo.stmt, assig)
        if not isinstance(p, Compound):
            continue

        parent_p = find_parentUpdated(while_algo.stmt, p)

        ind = p.block_items.index(assig)
        del p.block_items[ind]

        if not isinstance(parent_p, If):
            raise ValueError("Phase jumps are implemented only under ifs")

        parent_parent_p = find_parent(while_algo.stmt, parent_p)

        if not isinstance(parent_parent_p, Compound):
            continue

        # Check not to have code after this compound
        aux_parent = parent_parent_p
        old_parent = find_parentUpdated(outer_while, parent_parent_p)
        while old_parent != outer_while:
            if not isinstance(old_parent, Compound):
                aux_parent = old_parent
                old_parent = find_parentUpdated(outer_while, aux_parent)
                continue
            # Last Compound will be repeated so force exit with exception
            try:
                ind = old_parent.block_items.index(aux_parent)
            except ValueError:
                break
            if ind + 1 < len(old_parent.block_items):
                raise_warning = True
                break
            else:
                aux_parent = old_parent
                old_parent = find_parentUpdated(outer_while, aux_parent)

        ind = parent_parent_p.block_items.index(parent_p)

        if_node = construct_if_cond_node("iter", "PHASE", my_coord)

        # Capture everything under if (iter == PHASE)
        nodes_list = []
        aux_ind = ind
        while ind < len(parent_parent_p.block_items):
            nodes_list.append(parent_parent_p.block_items[ind])
            ind += 1

        while aux_ind < len(parent_parent_p.block_items):
            del parent_parent_p.block_items[aux_ind]

        if_node.iftrue = Compound(nodes_list, my_coord)
        parent_parent_p.block_items.append(if_node)

        # Define if node which introduces nondeterministic execution
        cond = FuncCall(ID("rand_bool", parent_p.coord), None, my_coord)
        nondet_rand = FuncCall(ID("rand_int", parent_p.coord), None, my_coord)
        nondet_phase_assig = Assignment("=", ID("PHASE", my_coord),
                                        BinaryOp("+", ID("PHASE", my_coord), nondet_rand), my_coord)
        if_true_branch = Compound([nondet_phase_assig], my_coord)

        if_nondet = If(cond, if_true_branch, None, my_coord)

        parent_parent_p.block_items.insert(aux_ind, if_nondet)

    return raise_warning


def turn_nested_algo_marked_compound(main_func_node, syntax_dict, nested_algos_details, rounds_list,
                                     msg_structure_fields, outer_while):
    """
    Found that we have nested algorithms from the config file
    Iterate through every statement and check for non recv while loops

    For each non recv while loop, take the compound and add markers before
    and after it
    :return: True/False if there is something not implemented discovered
    """
    v = WhileAlgoVisitor(rounds_list, msg_structure_fields)
    v.visit(main_func_node)
    raise_warning = False
    for el in v.result_list:
        parent = find_parent(main_func_node, el)

        elem = el

        # Suppose that nested algos start only in compounds
        if not isinstance(parent, Compound):
            continue
        ind = parent.block_items.index(elem)
        rank = WhileAlgoVisitor.get_rank_of_algo(elem.stmt, rounds_list, msg_structure_fields)

        if rank == -1:
            continue

        # If the algorithm has UPON syntax, then extract the possible init section
        if syntax_dict[rank]:
            to_add = get_init_section(main_func_node, elem, msg_structure_fields[rank]["round_field"],
                                      rounds_list[rank][0])

            if len(to_add) > 0:
                for init_stm in to_add:
                    elem.stmt.block_items.insert(0, init_stm)

        # Check if this algo has phase jumps
        (result, assig_list) = check_phase_jump(elem, rank, msg_structure_fields)

        if result:
            aux_warn = update_code_nondet(elem, assig_list, outer_while)
            if aux_warn:
                raise_warning = aux_warn

        #isolate_jump_phase(assig_list, main_func_node)

        # Add start/stop markers
        new_id = ID("marker_start", elem.coord)
        func = FuncCall(new_id, None, elem.coord)

        elem.stmt.block_items.insert(0, func)

        new_id = ID("marker_stop", elem.coord)
        func = FuncCall(new_id, None, elem.coord)

        elem.stmt.block_items.append(func)

        # Consider that 0 is the most outer algorithm
        # Paths extracting is based on outer while (true)
        # so that has to stay
        if rank == 0:
            nested_algos_details.append((rank, elem.stmt, parent, result))
            continue
        del parent.block_items[ind]

        siz = len(elem.stmt.block_items)
        j = 0
        while j < siz:
            parent.block_items.insert(ind, elem.stmt.block_items[j])
            ind += 1
            j += 1
        nested_algos_details.append((rank, elem.stmt, parent, result))

    return raise_warning


def add_to_param_list(ast, decl_vars, all_vars, mbox_name):
    """
    This function will use a visitor for each AST in trees_dict
    :param ast:
    :param decl_vars:
    :param all_vars:
    :param mbox_name:
    :return:
    """
    m_body = get_extern_while_body_from_func(ast, "main")

    v = DeclAlgoVisitor(mbox_name)
    v.visit(m_body)

    decl_vars.extend(list(map(lambda x: x[0], v.result_list)))

    v = AllVarsAlgoVisitor()
    v.visit(m_body)

    all_vars.extend(v.result_list)


def get_param_list(trees_dict, i, global_vars, mbox_name, vars_table, mailbox, msg_structure_fields, rounds_list):

    # Construct the list of declared variables and of all used vars
    decl_var = []
    all_vars = []
    for elem in trees_dict.values():
        for subel in elem:
            add_to_param_list(subel, decl_var, all_vars, mbox_name)

    # Get unique elements
    decl_var = list(set(decl_var))
    all_vars = list(set(all_vars))

    # Keep elems which are not decl or global
    all_vars = list(filter(lambda x: (x not in decl_var) and (x not in global_vars), all_vars))

    all_vars = list(filter(lambda x: (x != "pid") and (x != "n") and (x != "ROUND") and (x != "PHASE"), all_vars))

    # Eliminate own algo variables
    all_vars = list(filter(lambda x: (x != mailbox[i]) and (x != msg_structure_fields[i]["name"]),
                           all_vars))

    # Consider known all phases and rounds
    for el in msg_structure_fields:
        all_vars = list(filter(lambda x: (x != el["phase_field"]) and (x != el["round_field"]), all_vars))

    # Eliminate labels of algorithms
    for j in rounds_list:
        all_vars = list(filter(lambda x: (x not in j), all_vars))

    final_list = []
    for el in all_vars:
        if el in vars_table:
            final_list.append(vars_table[el][0] + " " + el)
        elif "old_" in el:
            for mbox in mailbox:
                if mbox in el:
                    if mbox in vars_table:
                        final_list.append(vars_table[mbox][0] + " " + el)
                        break

    return final_list


def turn_send_loops_funcs(x, rounds_list, msg_structure_fields):
    v = SendLoopsVisitor(rounds_list, msg_structure_fields)
    v.visit(x)

    for el in v.result_list:
        p = find_parent(x, el)

        if p and isinstance(p, Compound):
            ind = p.block_items.index(el)

            del p.block_items[ind]
            new_id = ID("send_mbox", p.coord)
            func = FuncCall(new_id, None, p.coord)
            p.block_items.insert(ind, func)
