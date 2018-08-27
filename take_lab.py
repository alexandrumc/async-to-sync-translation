from pycparser import parse_file, c_parser, c_generator, c_ast
from parse_test import get_label, duplicate_element, prune_tree, get_label_assign_num, find_all_paths_to_label_modified,\
    TreeGenerator, generate_c_code_from_paths_and_trees, RoundGenerator, find_parent
import copy
from pycparser.c_ast import While, Assignment, ID, If, Node, FuncDef, FileAST, Constant, UnaryOp, Compound, FuncCall, BinaryOp, StructRef, ArrayRef, \
    For

generator = c_generator.CGenerator()





def conds_to_source_and_dest(current_node, lab_source, lab_dest, destination_reached, source_reached,to_source, to_dest):
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
                conds_to_source_and_dest(child.iftrue, lab_source, lab_dest, destination_reached, source_reached, to_source,to_dest)
                if source_reached:
                    child.iffalse = None
                elif child.iffalse is not None:
                    child.iftrue = None
                    conds_to_source_and_dest(child.iffalse, lab_source, lab_dest, destination_reached, source_reached, to_source, to_dest)
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
                    to_dest.append(child.cond)
                    conds_to_source_and_dest(child.iftrue, lab_source, lab_dest, destination_reached, source_reached,to_source, to_dest)
                    if destination_reached:
                        child.iffalse = None
                    elif child.iffalse is not None:
                        conds_to_source_and_dest(child.iffalse, lab_source, lab_dest, destination_reached, source_reached, to_source, to_dest)
                        if destination_reached:
                            child.iftrue = None
                    continue
            else:
                to_delete.append(child)
    for node in to_delete:
        current_node.block_items.remove(node)






def remove_numbers(string):
    no_digits = []
    # Iterate through the string, adding non-numbers to the no_digits list
    for i in string:
        if not i.isdigit():
            no_digits.append(i)
        else:
            index = string.index(i)
            i = int(i) + 1
            no_digits.append(str(i))
    result = ''.join(no_digits)
    return result, index


def get_extern_while_body_from_func(ast, func_name):
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

    return labels


def get_labels_order(filename, labelname):
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

    return aux_list


def get_paths_trees(ast, labels,labels_sorted, labelname):
    trees_dict = {}
    trees_paths_dict = {}
    for label1 in labels_sorted:
        trees_list = []
        trees_paths_list = []
        labels_start = get_label(ast, labelname, label1)

        for label2 in labels_sorted[labels_sorted.index(label1) + 1:]:

            labels_end = get_label(ast, labelname, label2)

            for start in labels_start:
                #la final sa bag conditiile pt urmatorul element de iterat
                for end in labels_end:
                    cop = duplicate_element(ast)
                    # prune_tree(get_extern_while_body_from_func(cop),start,end,[],[])

                    dest_list = []
                    source_list = []
                    prune_tree(get_extern_while_body_from_func(cop, 'main'), start, end, dest_list, source_list)
                    # to_source = []
                    # to_dest = []
                    # conds_to_source_and_dest(get_extern_while_body_from_func(cop, 'main'), start, end, dest_list, source_list,to_source, to_dest)
                    # aux = find_all_paths_to_label_modified(cop,start,end)
                    # test = take_the_first(aux, labelname)
                    # print len(test)
                    # trees_paths_list.append(test)
                    #
                    if dest_list and source_list:
                        assign = get_label_assign_num(cop, labelname)
                        if assign <= 2:
                            trees_list.append(cop)
                            # if to_dest:
                            #     print label1, label2, "doar din prune"
                            #     for x in to_dest:
                            #         print generator.visit(x)

                        else:
                            # print start, end
                            if labels != labels_sorted:
                                aux = find_all_paths_to_label_modified(cop, start, end)
                                test = take_the_first(aux, labelname)
                                # print type(test), len(test)
                                # trees_paths_list.append(aux)
                                if test:
                                    trees_paths_list.append(test)
                                    # if to_dest:
                                    #     print label1, label2, "cu drumurile"
                                    #     for x in to_dest:
                                    #         print generator.visit(x)



        trees_dict[label1] = trees_list
        trees_paths_dict[label1] = trees_paths_list
        # break
    return trees_dict, trees_paths_dict


def modify_cond(cond, new_cond):


    if isinstance(cond, ID):
        if "pid" not in cond.name:
            aux = new_cond + cond.name
            cond.name = aux

    elif isinstance(cond,UnaryOp):
        modify_cond(cond.expr, new_cond)
    elif isinstance(cond.left, StructRef):
        if isinstance(cond.left.name, ID):
            if "pid" not in cond.left.name.name:
                aux = new_cond + cond.left.name.name
                cond.left.name.name = aux
        if isinstance(cond.left.name, ArrayRef):
            array = cond.left.name
            if isinstance(array, ArrayRef):
                if "pid" not in array.name.name:
                    aux = new_cond + array.name.name
                    array.name.name = aux
            else:
                if "pid" not in array.name.name.name:
                    aux = new_cond + array.name.name.name
                    array.name.name.name = aux

    elif isinstance(cond.left, ID):
        if "pid" not in cond.left.name:
            aux = new_cond + cond.left.name
            cond.left.name = aux

    elif isinstance(cond, BinaryOp):
        if isinstance(cond.left, BinaryOp):
            modify_cond(cond.left, new_cond)
        if isinstance(cond.right, BinaryOp):
            modify_cond(cond.right, new_cond)


def take_cond_name(cond, label, lista):
    """
    construieste lista cu variabile care sunt de adaugat
    primeste label pentru a stii in ce runda sunt si pentru a stii ce nume pun la variabila
    :param cond:
    :param label:
    :param lista:
    :return:
    """
    strings = ['pid', 'old', 'timeout']
    if isinstance(cond,UnaryOp):
        take_cond_name(cond.expr,label,lista)

    elif isinstance(cond, ID):
        if not any(x in cond.name for x in strings):
            aux = label + cond.name
            if aux not in lista:
                lista.append(aux)

    elif isinstance(cond.left, StructRef):
        if isinstance(cond.left.name, ID):
            if not any(x in cond.left.name.name for x in strings):
                aux = label + cond.left.name.name
                if aux not in lista:
                    lista.append(aux)

        if isinstance(cond.left.name, ArrayRef):
            array = cond.left.name
            if isinstance(array,ArrayRef):
                if not any(x in array.name.name for x in strings):
                    aux = label + array.name.name
                    if aux not in lista:
                        lista.append(aux)
            else:
                print type(array)
                if not any(x in array.name.name.name for x in strings):
                    aux = label + array.name.name.name
                    if aux not in lista:
                        lista.append(aux)

    elif isinstance(cond.left, ID):
        if not any(x in cond.left.name for x in strings):
            aux = label + cond.left.name
            if aux not in lista:
                lista.append(aux)

    elif isinstance(cond, BinaryOp):
        if isinstance(cond.left, BinaryOp):
            take_cond_name(cond.left, label, lista)
        if isinstance(cond.right, BinaryOp):
            take_cond_name(cond.right, label, lista)


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
            add_assign_in_tree(get_extern_while_body_from_func(trees_list[i], 'main'), x, [], original_ast)


def get_code_from_trees_only(trees_dict, labels):
    code = {}
    gen = TreeGenerator()
    for x in labels:
        code_for_label = []
        trees_list = trees_dict[x]
        # print x
        for tree in trees_list:
            code_for_label.append(gen.visit(get_extern_while_body_from_func(tree, 'main')))
            print gen.visit(get_extern_while_body_from_func(tree, 'main'))

        code[x] = code_for_label
    return code


def take_the_first(paths, labelname):
    aux = []
    for tuple in paths:
        tree = tuple[0]
        path = tuple[1]
        assign = 0
        for node in path:
            if isinstance(node, Assignment) and node.lvalue.name == labelname:
                assign +=1

        if assign > 2:
            aux.append(tuple)

    for x in aux:
        paths.remove(x)
    return paths




def remove_bad_paths(trees_paths_dict, labels, labelname):
    for x in labels:
        trees_paths_list = trees_paths_dict[x]
        for i  in xrange(len(trees_paths_list)):
            list_aux = trees_paths_list[i]
            to_delete = []
            for tuple in list_aux:
                tree = tuple[0]
                path = tuple[1]
                assign = 0
                for node in path:
                    if isinstance(node,Assignment) and node.lvalue.name == labelname:
                        assign += 1
                if assign > 2:
                    to_delete.append(tuple)
            for elem in to_delete:
                list_aux.remove(elem)



def print_code_from_trees_paths(trees_paths_dict, labels):
    for x in labels:
        paths = trees_paths_dict[x]
        for path in paths:
            if path is not None:
                generate_c_code_from_paths_and_trees(path)

def print_code_from_trees(trees_dict,labels):
    gen = TreeGenerator()
    for x in labels:
        trees_list = trees_dict[x]
        for tree in trees_list:
            print gen.visit(get_extern_while_body_from_func(tree, 'main'))

def print_code(trees_dict, trees_paths_dict, labels):
    print_code_from_trees(trees_dict, labels)
    print_code_from_trees_paths(trees_paths_dict, labels)


def print_rounds(labels, trees_dict, trees_paths_dict):
    for label in labels[:len(labels) - 1]:
        print "def round " + label + ":"

        print "  SEND():"

        found_send_list = []
        history_of_strings = []

        for tree in trees_dict[label]:
            gen = RoundGenerator("send")
            gen.first_compound = False
            result = gen.visit(get_extern_while_body(tree))
            if gen.send_reached:
                if result not in history_of_strings:
                    print result
                    history_of_strings.append(result)
                found_send_list.append(True)
            else:
                found_send_list.append(False)

        list_of_lists_of_tuples = trees_paths_dict[label]
        for list_of_tuples in list_of_lists_of_tuples:
            tuple_el = list_of_tuples[0]
            gen = RoundGenerator("send", tuple_el[1])
            gen.first_compound = False
            result = gen.visit(get_extern_while_body(tuple_el[0]))
            if gen.send_reached:
                if result not in history_of_strings:
                    print result
                    history_of_strings.append(result)
                found_send_list.append(True)
            else:
                found_send_list.append(False)

        print "  UPDATE():"
        history_of_strings = []
        for i, tree in enumerate(trees_dict[label]):
            gen = RoundGenerator("update")
            gen.first_compound = False
            if not found_send_list[i]:
                gen.send_reached = True
            result = gen.visit(get_extern_while_body(tree))
            if result not in history_of_strings:
                print result
                history_of_strings.append(result)

        i = len(trees_dict[label])
        for list_of_tuples in list_of_lists_of_tuples:
            tuple_el = list_of_tuples[0]
            gen = RoundGenerator("update", tuple_el[1])
            gen.first_compound = False
            if not found_send_list[i]:
                gen.send_reached = True
            result = gen.visit(get_extern_while_body(tuple_el[0]))
            if result not in history_of_strings:
                print result
                history_of_strings.append(result)
            i = i + 1


def take_code_from_file(ast, filename, labelname):
    x = copy.deepcopy(ast)
    labels_sorted = get_labels_order(filename, labelname)
    labels = get_labels(filename,labelname)

    trees_dict, trees_paths_dict = get_paths_trees(ast, labels,labels_sorted, labelname)

    #print_code(trees_dict,trees_paths_dict,labels_sorted)

    print_rounds(labels_sorted, trees_dict, trees_paths_dict)

    return trees_dict
