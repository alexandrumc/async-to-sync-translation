from parse_test import *
from pycparser.c_ast import *

generator = c_generator.CGenerator()


def remove_mbox_assign_to_zero(extern_while_body):
    to_delete = []
    for elem in extern_while_body.block_items:
        if isinstance(elem, Assignment) and "mbox" in elem.lvalue.name and int(elem.rvalue.value) == 0:
            to_delete.append(elem)
        if isinstance(elem, If):
            remove_mbox_assign_to_zero(elem.iftrue)
            if elem.iffalse:
                remove_mbox_assign_to_zero(elem.iffalse)

    for x in to_delete:
        extern_while_body.block_items.remove(x)


def remove_mbox_free(extern_while_body):
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
    remove_mbox_assign_to_zero(extern_while_body)
    remove_mbox_free(extern_while_body)


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
                for end in labels_end:
                    cop = duplicate_element(ast)
                    # prune_tree(get_extern_while_body_from_func(cop),start,end,[],[])

                    dest_list = []
                    source_list = []
                    prune_tree(get_extern_while_body_from_func(cop, 'main'), start, end, dest_list, source_list)

                    if dest_list and source_list:
                        assign = get_label_assign_num(cop, labelname)
                        if assign <= 2:
                            trees_list.append(cop)
                        else:
                            # print start, end
                            if labels != labels_sorted:
                                aux = find_all_paths_to_label_modified(cop, start, end)
                                trees_paths_list.append(aux)

        trees_dict[label1] = trees_list
        trees_paths_dict[label1] = trees_paths_list
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


def print_code_from_trees_paths(trees_paths_dict, labels):
    for x in labels:
        paths = trees_paths_dict[x]
        for path in paths:
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


def print_rounds(labels, trees_dict):
    for label in labels[:len(labels) - 1]:
        print "def round " + label + ":"
        print "  SEND():"
        for tree in trees_dict[label]:
            print RoundGenerator("send").visit(get_extern_while_body_from_func(tree, 'main'))
        print "  UPDATE():"
        for tree in trees_dict[label]:
            print RoundGenerator("update").visit(get_extern_while_body_from_func(tree, 'main'))


def take_code_from_file(ast, filename, labelname):
    x = copy.deepcopy(ast)
    labels_sorted = get_labels_order(filename, labelname)
    labels = get_labels(filename,labelname)
    # print labels
    trees_dict, trees_paths_dict = get_paths_trees(ast, labels,labels_sorted, labelname)
    # add_ghost_assign(trees_dict, labels, ast)
    # trees_dict, trees_paths_dict = get_paths_trees(ast, labels,labels_sorted, labelname)
    # code = get_code_from_trees_only(trees_dict, labels)
    # print_code_from_trees_paths(trees_paths_dict, labels)
    # print_rounds(labels,trees_dict)
    print_code(trees_dict,trees_paths_dict,labels)
    return trees_dict
