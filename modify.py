import copy

from take_lab import *
# from take_old import *

# from cod_alex import show_func_calls
from parse_test import RoundGenerator

from modify_whiles import *
generator = c_generator.CGenerator()
# print(generator.visit(ast))

# main_function = ast.ext[1]




#
ast = parse_file(filename="examples/c_files/ct-non-terminating.c", use_cpp=False)
extern_while_body = None

x = get_extern_while_body(ast)
#
whiles_to_if(x)
# print generator.visit(x)
# for item in x:
#     if isinstance(item,While):
#         print generator.visit(item)
        # break
# print generator.visit(item)
# print to_modify(item)

copie = copy.deepcopy(ast)

labels = get_labels("examples/c_files/ct-non-terminating.c", "round")
# print labels
trees_dict, code = take_code_from_file(copie,"examples/c_files/ct-non-terminating.c","round")
# print generator.visit(copie)




