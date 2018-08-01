from parse_test import *
from pycparser.c_ast import *

generator = c_generator.CGenerator()


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


def add_assign_in_tree(tree, change_conds=False):
    to_add = []
    # list = tree.children()
    if tree is not None:  # nu am idee unde ajunge aici pe None
        for i, item in enumerate(tree.block_items):
            if isinstance(item, If):
                if isinstance(item.cond, BinaryOp):

                    if isinstance(item.cond.left, StructRef):
                        old_var = "old_" + item.cond.left.name.name

                        assign = Assignment("=", ID(old_var), ID(item.cond.left.name.name), item.coord)

                        if change_conds:
                            item.cond.left.name.name = old_var

                        to_add.append((i + len(to_add), assign))
                        # pun + len(to_add) pentru ca daca adaug prima data un assign la pozitia 5, practic pozitia
                        # se incrementeaza
                    else:
                        old_var = "old_" + item.cond.left.name
                        assign = Assignment("=", ID(old_var), ID(item.cond.left.name), item.coord)

                        if change_conds:
                            item.cond.left.name = old_var

                        to_add.append((i + len(to_add), assign))

                else:

                    old_var = "old_" + item.cond.name
                    # print "aaaa", type(item.cond.name)
                    assign = Assignment("=", ID(old_var), ID(item.cond.name), item.coord)

                    if change_conds:
                        item.cond.left.name = old_var

                    to_add.append((i + len(to_add), assign))

                add_assign_in_tree(tree.block_items[i].iftrue, change_conds)
                if tree.block_items[i].iffalse is not None:
                    add_assign_in_tree(tree.block_items[i].iffalse, change_conds)

    for index, element in to_add:
        tree.block_items.insert(index, element)


def add_assign_in_path(path, to_add):
    for i in xrange(len(path)):
        for k in to_add:
            if k[0] in path[i]:
                index = path[i].index(k[0])
                path[i].insert(index, k[1])


def add_ghost_assign(trees_dict,labels, change_conds=False):
    for x in labels:
        trees_list = trees_dict[x]
        for i in xrange(len(trees_list)):
            add_assign_in_tree(get_extern_while_body(trees_list[i]), change_conds)


def print_code_from_dicts(labels, trees_dict, paths_dict):
    for x in labels:
        trees = trees_dict[x]
        paths = paths_dict[x]
        # print len(trees), len(paths)
        for i in xrange(len(paths)):
            tree = trees[i]
            path = paths[i]
            generate_c_code_from_one_path(path, tree)


def print_code_from_trees_only(trees_dict,labels):
    code = {}
    gen = TreeGenerator()
    for x in labels:
        code_for_label  = []
        trees_list = trees_dict[x]
        # print x
        for tree in trees_list:
            code_for_label.append(gen.visit(get_extern_while_body(tree)))
            print gen.visit(get_extern_while_body(tree))

        code[x] = code_for_label
    return code

def take_code_from_file(ast, filename, labelname):
    labels = get_labels(filename, labelname)

    trees_dict = get_paths_trees(ast, labels, labelname)
    # add_ghost_assign(trees_dict, labels,True)
    code = print_code_from_trees_only(trees_dict,labels)
    return code
