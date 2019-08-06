"""
utils: utils.py

Module which contains utilities functions
"""

import copy
from pycparser.c_ast import FileAST, FuncDef, While, Decl, TypeDecl, PtrDecl
from generators import PathGenerator, LabelVisitor, LocateNode, LocateParentNode, CheckLabelNumber,\
    EpochVisitor, LocateParentNodeUpdated, SendWhileVisitor, RecvWhileVisitor, DeclAlgoVisitor

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
    if isinstance(ast, FileAST):
        for ext in ast.ext:
            if isinstance(ext, FuncDef) and ext.decl.name == main_function_name:
                return ext
    return ast


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


def get_recv_whiles(ast_tree, rounds_list, msg_fields):
    v = RecvWhileVisitor(msg_fields, rounds_list)
    v.visit(ast_tree)
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
