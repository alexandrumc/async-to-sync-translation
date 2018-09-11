import copy
from pycparser.c_ast import FileAST, FuncDef, While
from generators import PathGenerator, LabelVisitor, LocateNode, LocateParentNode, CheckLabelNumber

main_function_name = "main"
new_code = "\n\n\n\n NEW CODE \n\n\n\n"
new_path = "\n\n NEW PATH \n\n"
new_element = "\n\nNEW ELEMENT\n\n"


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


def get_label_assign_num(ast_tree, label_name):
    v = CheckLabelNumber(label_name)
    v.visit(ast_tree)
    return v.count_labels


def get_label(ast_tree, label_name, label_value):
    v = LabelVisitor(label_name, label_value)
    v.visit(ast_tree)
    return v.label_item


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
