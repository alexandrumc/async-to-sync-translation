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
ast = parse_file(filename="examples/c_files/broadcast.c", use_cpp=False)
extern_while_body = None

x = get_extern_while_body_from_func(ast, "main")
conditii = []
whiles_to_if(x, conditii)

identify_recv_exits(x, conditii)
remove_mbox(x)

test = get_labels_order("examples/c_files/broadcast.c",'round')
# print test
#print generator.visit(x)

take_code_from_file(ast,"examples/c_files/broadcast.c",'round')
# copie = copy.deepcopy(ast)



# print generator.visit(x)



