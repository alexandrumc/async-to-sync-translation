import copy

from take_lab import *
# from take_old import *
from incercare import take_paths
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

whiles_to_if(x)

identify_recv_exits(x)
remove_mbox(x)


copie = copy.deepcopy(ast)



print generator.visit(x)



