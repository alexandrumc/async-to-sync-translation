from pycparser.c_ast import *

from parse_test import *

generator = c_generator.CGenerator()


def read_from_two_lines(filename, line1, line2):
    with open(filename) as f:
        lines = f.readlines()
    x = lines[line1:line2 - 1]

    for i in x:
        t = i.replace(" ", "")
        if "\n" in t:
            t = t.replace("\n", "")
        x[x.index(i)] = t

    list = []
    for i in x:
        if i is not "":
            list.append(i)
    return list


def identify_changes(filename, labelname):
    lab = labelname + "="
    with open(filename) as f:
        lines = f.readlines()

    for i in lines:
        t = i.replace(" ", "")
        if "\n" in t:
            t = t.replace("\n", "")
        lines[lines.index(i)] = t

    list = []
    for i in lines:
        if i is not "":
            list.append(i)

    index = 0
    changes_indexes = []
    for i in list:
        if labelname in i:
            index += 1
            loc = list.index(i)
            changes_indexes.append(loc)
            list[loc] = list[loc].replace(lab, "")  # linia asta elimita "lab="

    changes = []
    for i in xrange(len(changes_indexes)):
        if i + 1 == len(changes_indexes):
            changes.append(list[changes_indexes[i]:])
        else:
            changes.append(list[changes_indexes[i]:changes_indexes[i + 1] + 1])
            # acest ultim +1 ia si schimbarea de label

    return list, changes_indexes, changes


def get_extern_while_body(ast):
    for ext in ast.ext:
        if isinstance(ext, FuncDef) and ext.decl.name == "main":
            amain_body = ext
            if amain_body is not None:
                for operation in amain_body.body:
                    # print type(operation)
                    if isinstance(operation, While):
                        return operation.stmt


def try_coord(lca, label1, label2, x, y):
    """
    returns the line numbers where labels are assigned
    all code between this 2 values should be collected
    :param lca: least common ancestor
    :param label1: first label
    :param label2: second albel
    :param x: line for first label
    :param y: line for second label
    :return: tuple line of the first label, line of the second label
    """
    for line in lca:
        if line == label1:
            x = line.coord.line
        if line == label2:
            y = line.coord.line
            break
        if isinstance(line, If):
            (x, y) = try_coord(line.iftrue, label1, label2, x, y)
            if line.iffalse is not None:
                (x, y) = try_coord(line.iffalse, label1, label2, x, y)
        if isinstance(line, Compound):
            (x, y) = try_coord(line, label1, label2, x, y)
    return (x, y)


def take_code_between_lines(lca, line1, line2):
    for operation in lca:
        if operation.coord == line1:
            print line1
        if operation.coord == line2:
            print line2
            break
        if isinstance(operation, Compound):
            take_code_between_lines(lca, line1, line2)


def print_code(code):
    for list in code:
        for each in list:
            print each


def get_labels(filename, labelname):
    labels = []
    lab = labelname + "="
    with open(filename) as f:
        lines = f.readlines()
    for each in lines:
        if labelname in each:
            aux = each.replace(" ", "")
            if "\n" in aux:
                aux = aux.replace("\n", "")
            aux = aux.replace(lab, "")
            aux = aux.replace(";", "")
            labels.append(aux)

    return labels


def get_paths_trees(ast, labels, labelname):
    paths = []
    # print keys
    nodes = []
    trees = []

    for i in xrange(len(labels)):
        if i + 1 < len(labels):
            label1 = labels[i]
            label2 = labels[i + 1]

            cop = copy.deepcopy(ast)
            trees.append(cop)

            node1 = get_label(trees[i], labelname, label1)
            node2 = get_label(trees[i], labelname, label2)
            aux = (node1,node2)
            nodes.append(aux)

    for i in xrange(len(labels)):
        if i + 1 < len(labels):
            label2 = labels[i + 1]
        else:
            break

        print labels[i], label2
        label1 = labels[i]
        prune_tree(get_extern_while_body(trees[i]), nodes[i][0], nodes[i][1], [], [])
        path = find_all_paths_to_label(trees[i], labelname, label1, label2)
        paths.append(path)

    return trees, paths


def take_code(trees, paths):
    code = []
    for i in xrange(len(paths)):
        cod = get_code_paths_list(paths[i], trees[i])
        code.append(cod)
    return code


def take_code_from_file(ast, filename, labelname):
    labels = get_labels(filename, labelname)

    trees, paths = get_paths_trees(ast, labels, labelname)

    code = take_code(trees, paths)

    return code


def cumulate_if(ast,list):

    if isinstance(ast,FileAST):
        for ext in ast.ext:
            if isinstance(ext, FuncDef) and ext.decl.name == "main":
                main_body = ext

        for operation in main_body.body:
            if isinstance(operation, If):
                list.append(operation.cond)
                cumulate_if(operation.iftrue,list)



    if isinstance(ast, Compound):
        # print "am compund"
        for line in ast.block_items:
            cumulate_if(line,list)

    if isinstance(ast, If):
        if isinstance(ast.iftrue.block_items[0], If):
            list.append(ast.cond)
            cumulate_if(ast.iftrue, list)
        else:
            return list



    return list

def combine_cond_from_list(list):
    left = list[0]
    for i in xrange(len(list)):
        if(i !=0):
            right = list[i]
            op = BinaryOp('&&',left,right)
            left = op
    return op