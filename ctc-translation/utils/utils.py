"""
utils: utils.py

Module which contains utilities functions
"""

import copy
from pycparser.c_ast import *
from generators import PathGenerator, LabelVisitor, LocateNode, LocateParentNode, CheckLabelNumber,\
    EpochVisitor, LocateParentNodeUpdated, SendWhileVisitor, RecvWhileVisitor, DeclAlgoVisitor, WhileAlgoVisitor

main_function_name = "main"
new_code = "\n\n\n\n NEW CODE \n\n\n\n"
new_path = "\n\n NEW PATH \n\n"
new_element = "\n\nNEW ELEMENT\n\n"


def get_global_vars(ast, result_list):
    if not ast:
        return
    for elem in ast.ext:
        if isinstance(elem, Decl) and len(elem.storage) == 0:
            if isinstance(elem.type, TypeDecl):
                result_list.append(elem.name)
            elif isinstance(elem.type, PtrDecl):
                result_list.append("*" + elem.name)


def get_vars_table(ast, vars_table):
    if not ast:
        return

    v = DeclAlgoVisitor("")
    v.visit(ast)

    for el in v.result_list:
        if el[1] != "":
            if el[0] in vars_table:
                if el[1] not in vars_table[el[0]]:
                    vars_table[el[0]].append(el[1])
            else:
                vars_table[el[0]] = [el[1]]


def duplicate_element(element):
    return copy.deepcopy(element)


def get_extern_while_body(ast):
    if isinstance(ast, FileAST):
        for ext in ast.ext:
            if isinstance(ext, FuncDef) and ext.decl.name == main_function_name:
                main_body = ext.body
                for operation in main_body:
                    if isinstance(operation, While):
                        return operation.stmt
    return ast


def get_extern_while(ast):
    if isinstance(ast, FileAST):
        for ext in ast.ext:
            if isinstance(ext, FuncDef) and ext.decl.name == main_function_name:
                main_body = ext.body
                for operation in main_body:
                    if isinstance(operation, While):
                        return operation
    return ast


def get_main_function(ast):
    if not isinstance(ast, FileAST):
        raise ValueError('Incorrect argument')

    l = ast.children()
    i = 0
    while i < len(l):
        if isinstance(l[i][1], FuncDef) and l[i][1].decl.name == main_function_name:
            break
        i += 1
    return l[i][1]


def get_init_section(ast, node, round_label, round_name):
    """
    Searches for a round assignment above the while algorithm, but in the
    same compound as the while. Useful for UPON syntax, as this one usually
    has a few init statements
    :param ast:
    :param node:
    :param round_label:
    :param round_name:
    :return: A list of nodes
    """

    p = find_parent(ast, node)

    if not isinstance(p, Compound):
        return []

    result = []

    ind_while = p.block_items.index(node)
    i = ind_while - 1

    while i >= 0:
        if isinstance(p.block_items[i], Assignment):
            if isinstance(p.block_items[i].lvalue, ID) and p.block_items[i].lvalue.name == round_label:
                if isinstance(p.block_items[i].rvalue, ID) and p.block_items[i].rvalue.name == round_name:
                    result.append(p.block_items[i])
                    break
        result.append(p.block_items[i])
        i -= 1

    if i < 0:
        return []
    result.reverse()

    ind_assig = p.block_items.index(result[0])
    i = ind_assig
    while i < ind_while:
        del p.block_items[ind_assig]
        i += 1
    result.reverse()
    return result


def find_node(ast_tree, node):
    v = LocateNode(node)
    v.visit(ast_tree)
    return v.discovered_node


def find_parent(ast_tree, child_node):
    v = LocateParentNode(child_node)
    v.visit(ast_tree)
    return v.discovered_node


def find_parentUpdated(ast_tree, child_node):
    v = LocateParentNodeUpdated(child_node)
    v.visit(ast_tree)
    return v.discovered_node


def get_label_assign_num(ast_tree, label_name):
    v = CheckLabelNumber(label_name)
    v.visit(get_extern_while_body(ast_tree))
    return v.count_labels


def get_label(ast_tree, label_name, label_value):
    v = LabelVisitor(label_name, label_value)
    v.visit(ast_tree)
    return v.label_item


def get_epochs_assigns(ast_tree, epoch_name):
    v = EpochVisitor(epoch_name)
    v.visit(ast_tree)
    return v.epoch_list


def get_send_usage(ast_tree):
    v = SendWhileVisitor()
    v.visit(ast_tree)
    return v.list


def is_upon_syntax(node, msg_fields, rounds_list):
    """
    Wrapper for get_syntax_of_algo
    :param node: while algo node
    :param msg_fields: msg_struct_fields from config
    :param rounds_list: rounds_list from config
    :return: True for UPON, False otherwise
    """

    try:
        value = get_syntax_of_algo(node, msg_fields, rounds_list)
        if value == 1:
            return True
        return False
    except ValueError:
        # Unexpected args
        return False


def get_syntax_of_algo(node, msg_fields, rounds_list):
    """
    It determines if the given while algo node has UPON syntax or not
    :param node: while algo node
    :param msg_fields: msg_struct_fields from config
    :param rounds_list: rounds_list from config
    :return: 0 for old syntax, 1 for UPON syntax, raises ValueError for incorrect args
    """

    if not isinstance(node, While):
        raise ValueError("Given argument can't be while algo")

    v = RecvWhileVisitor(msg_fields, rounds_list)
    v.visit(node)
    count = len(v.recv_loops)

    if count != 1:
        return 0

    if node.stmt and isinstance(node.stmt, Compound):
        if node.stmt.block_items and len(node.stmt.block_items) > 0:
            if isinstance(node.stmt.block_items[0], While):
                if WhileAlgoVisitor.check_if_recv_loop(node.stmt.block_items[0].stmt):
                    return 1
            elif isinstance(node.stmt.block_items[0], Assignment) and isinstance(node.stmt.block_items[1], While):
                if WhileAlgoVisitor.check_if_recv_loop(node.stmt.block_items[0].stmt):
                    return 1

    if node.stmt and isinstance(node.stmt, While):
        if WhileAlgoVisitor.check_if_recv_loop(node.stmt.stmt):
            return 1

    return 0


def get_recv_whiles(ast_tree, rounds_list, msg_fields, recv_loops):
    v = RecvWhileVisitor(msg_fields, rounds_list)
    v.visit(ast_tree)
    recv_loops.update(v.recv_loops)
    return v.result


def print_path(paths_list, index=-1):
    if index < 0:
        for path in paths_list:
            print new_path
            for element in path:
                print new_element
                print element
    else:
        for element in paths_list[index]:
            print new_element
            print element


def generate_c_code_from_paths(paths_list, ast):
    for path in paths_list:
        gen = PathGenerator(path)
        print new_code
        print(gen.visit(get_extern_while(ast)))


def generate_c_code_from_one_path(path, ast):
    gen = PathGenerator(path)
    print new_code
    print(gen.visit(get_extern_while(ast)))


def generate_c_code_from_paths_and_trees(tuples):
    for pair in tuples:
        gen = PathGenerator(pair[1])
        print new_code
        print(gen.visit(pair[0]))


def get_code_paths_list(paths_list, ast):
    result = []
    for path in paths_list:
        gen = PathGenerator(path)
        result.append(gen.visit(get_extern_while(ast)))
    return result


def find_lca(root, label1, label2):
    """
    Lowest common ancestor algorithm implementation.
    """
    if root is None:
        return None

    if root is label1 or root is label2:
        return root

    count = 0
    save_child = None

    for pair in root.children():
        child = pair[1]
        return_val = find_lca(child, label1, label2)
        if return_val is not None:
            save_child = return_val
            count += 1
        if count == 2:
            return root

    return save_child