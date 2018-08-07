from parse_test import *
from pycparser.c_ast import *

generator = c_generator.CGenerator()


# coord = [0]

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


def get_extern_while_body(ast):
    for ext in ast.ext:
        if isinstance(ext, FuncDef) and ext.decl.name == "main":
            amain_body = ext
            if amain_body is not None:
                for operation in amain_body.body:
                    if isinstance(operation, While):
                        return operation.stmt


def take_code_between_lines(lca, line1, line2):
    for operation in lca:
        if operation.coord == line1:
            print line1
        if operation.coord == line2:
            print line2
            break
        if isinstance(operation, Compound):
            take_code_between_lines(lca, line1, line2)


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


def get_paths_trees(ast, labels, labelname):
    trees_dict = {}
    paths_dict = {}

    for label1 in labels:
        trees_list = []
        labels_start = get_label(ast, labelname, label1)
        source_to_all_trees = []
        source_to_all_paths = []

        for label2 in labels[labels.index(label1) + 1:]:
            # print label1, label2

            labels_end = get_label(ast, labelname, label2)
            # source_to_one_paths = []
            # source_to_one_trees = []
            for start in labels_start:
                for end in labels_end:
                    cop = duplicate_element(ast)
                    # prune_tree(get_extern_while_body(cop),start,end,[],[])

                    dest_list = []
                    source_list = []
                    prune_tree(get_extern_while_body(cop), start, end, dest_list, source_list)

                    if dest_list and source_list:
                        assign = get_label_assign_num(cop, labelname)
                        if assign <= 2:
                            trees_list.append(cop)
                        else:
                            pass

                        # print "copacii de la label ",label1, "pana la ", label2
                        # source_to_one_tuples = find_all_paths_to_label_modified(cop,start,end)
                        # source_to_one_paths = []
                        # source_to_one_trees = []

                        # for tup in source_to_one_tuples:
                        #     source_to_one_trees.append(tup[0])
                        #     source_to_one_paths.append(tup[1])
                    # source_to_all_paths.append(source_to_one_paths)
                    # source_to_all_trees.append(source_to_one_trees)

        trees_dict[label1] = trees_list

    return trees_dict


def remove_bad_paths(labels, paths_dict, labelname):
    for x in labels:
        paths = paths_dict[x]
        to_remove = []

        for i in xrange(len(paths)):
            path = paths[i]
            num = 0
            for elem in path:
                # print "elem : \n", elem
                # num = 0
                if isinstance(elem, Assignment) and elem.lvalue.name == labelname:
                    num += 1
            if num > 2:
                # print "am de sters"
                to_remove.append(path)
        for z in to_remove:
            paths.remove(z)


def modify_cond(cond, new_cond):
    if isinstance(cond, ID):
        if "pid" not in cond.name:
            aux = new_cond + cond.name
            cond.name = aux

    elif isinstance(cond.left, StructRef):
        if isinstance(cond.left.name, ID):
            if "pid" not in cond.left.name.name:
                aux = new_cond + cond.left.name.name
                cond.left.name.name = aux
        if isinstance(cond.left.name, ArrayRef):
            array = cond.left.name
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
    if isinstance(cond, ID):
        if "pid" not in cond.name:
            if "old" not in cond.name:
                aux = label + cond.name
                if aux not in lista:
                    lista.append(aux)

    elif isinstance(cond.left, StructRef):
        if isinstance(cond.left.name, ID):
            if "pid" not in cond.left.name.name:
                if "old" not in cond.left.name.name:
                    aux = label + cond.left.name.name
                    if aux not in lista:
                        lista.append(aux)
        if isinstance(cond.left.name, ArrayRef):
            array = cond.left.name
            if "pid" not in array.name.name.name:
                if "old" not in array.name.name.name:
                    aux = label + array.name.name.name
                    if aux not in lista:
                        lista.append(aux)

    elif isinstance(cond.left, ID):
        if "pid" not in cond.left.name:
            if "old" not in cond.left.name:
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
                add_assign_in_tree(item.stmt,label,variabile_old,original_ast)
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
                    elif isinstance(node.block_items[index - 1], Assignment) \
                            and "mbox" not in node.block_items[index - 1].lvalue.name:
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


def print_code_from_dicts(labels, trees_dict, paths_dict):
    for x in labels:
        trees = trees_dict[x]
        paths = paths_dict[x]
        # print len(trees), len(paths)
        for i in xrange(len(paths)):
            tree = trees[i]
            path = paths[i]
            generate_c_code_from_one_path(path, tree)


def print_code_from_trees_only(trees_dict, labels):
    code = {}
    gen = TreeGenerator()
    for x in labels:
        code_for_label = []
        trees_list = trees_dict[x]
        # print x
        for tree in trees_list:
            code_for_label.append(gen.visit(get_extern_while_body(tree)))
            # print gen.visit(get_extern_while_body(tree))

        code[x] = code_for_label
    return code


def print_rounds(labels, trees_dict):
    for label in labels[:len(labels) - 1]:
        print "def round " + label + ":"
        print "  SEND():"
        for tree in trees_dict[label]:
            print RoundGenerator("send").visit(get_extern_while_body(tree))
        print "  UPDATE():"
        for tree in trees_dict[label]:
            print RoundGenerator("update").visit(get_extern_while_body(tree))


def take_code_from_file(ast, filename, labelname):
    x = copy.deepcopy(ast)
    labels = get_labels(filename, labelname)

    trees_dict = get_paths_trees(ast, labels, labelname)
    add_ghost_assign(trees_dict, labels, ast)
    trees_dict = get_paths_trees(ast, labels, labelname)
    code = print_code_from_trees_only(trees_dict, labels)
    print_rounds(labels,trees_dict)
    return trees_dict, code
