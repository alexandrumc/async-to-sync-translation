import copy

from take_lab import *
# from take_old import *

# from cod_alex import show_func_calls
from parse_test import RoundGenerator


generator = c_generator.CGenerator()
# print(generator.visit(ast))

# main_function = ast.ext[1]




#
ast = parse_file(filename="examples/c_files/tpc_AMIT_modificat.c", use_cpp=False)
extern_while_body = None

x = get_extern_while_body(ast)

whiles_to_if(x)
# print generator.visit(ast)


copie = copy.deepcopy(ast)

labels = get_labels("examples/c_files/tpc_AMIT_modificat.c", "lab")
trees_dict, code = take_code_from_file(copie,"examples/c_files/tpc_AMIT_modificat.c","lab")


def find_indent(str):
    count = 0
    for char in str:
        if char == ' ':
            count += 1
        else:
            break
    return count / 2


def make_indent(s, times=1):
    for i in xrange(1, times):
        s = "  " + s
    return s


def print_send(line_list):
    result = ""
    parentheses_opened = []
    index = -1
    for line in line_list:
        if '{' in line:
            parentheses_opened.append(find_indent(line))
        if '}' in line:
            parentheses_opened.pop()
        result += make_indent(line, 2)
        result += "\n"
        if "send" in line:
            index = line_list.index(line) + 1
            break
    if index != -1:
        while parentheses_opened:
            indent = parentheses_opened.pop()
            result += make_indent("}", indent + 2)
            result += "\n"
        print result
    return index


def print_update(line_list, index):
    if index != -1:
        line_list = line_list[index:]
    result = ""
    if '}' in line_list[0]:
        #print "\n\n\nDA\n\n\n"
        line_list = line_list[1:]
    parentheses_opened = []
    for line in line_list:
        if '{' in line:
            parentheses_opened.append(find_indent(line))
        if '}' in line:
            if parentheses_opened:
                parentheses_opened.pop()
            else:
                continue
        result += make_indent(line, 2)
        result += "\n"
    while parentheses_opened:
        indent = parentheses_opened.pop()
        result += make_indent("}", indent + 2)
        result += "\n"
    print result

"""
for label in labels[:len(labels) - 1]:
    start_indexes_of_update = []
    print "def round " + label + ":"
    print "  SEND():"
    for string in code[label]:
        new_list = string.split('\n')
        ind = print_send(new_list)
        start_indexes_of_update.append(ind)
    print "  UPDATE():"
    for i in xrange(len(code[label])):
        string = code[label][i]
        new_list = string.split('\n')
        print_update(new_list, start_indexes_of_update[i])
"""


for label in labels[:len(labels) - 1]:
    print "def round " + label + ":"
    print "  SEND():"
    for tree in trees_dict[label]:
        print RoundGenerator("send").visit(get_extern_while_body(tree))
    print "  UPDATE():"
    for tree in trees_dict[label]:
        print RoundGenerator("update").visit(get_extern_while_body(tree))

#print "\n\nUPDATE\n\n"
#print generator.visit(get_extern_while_body(trees_dict["SECOND_ROUND"][0]))
#print RoundGenerator("update").visit(get_extern_while_body(trees_dict["FIRST_ROUND"][0]))