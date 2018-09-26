from take_lab import take_code_from_file, get_extern_while_body_from_func
from pycparser import parse_file

from modify_whiles import *
import os
generator = c_generator.CGenerator()




#
ast = parse_file(filename="../examples/c_files/two_algorithms.c", use_cpp=False)
extern_while_body = None

x = get_extern_while_body_from_func(ast, "main")
conditii = []
whiles_to_if(x, conditii)

identify_recv_exits(x, conditii)
remove_mbox(x)
#print generator.visit(x)
#test = get_labels_order("examples/c_files/broadcast.c",'round')
# print test
# print generator.visit(x)


# print generator.visit(test)

# print generator.visit(funcdef)

# print generator.visit(test)

take_code_from_file(ast,"../examples/c_files/two_algorithms.c",'round')
# copie = copy.deepcopy(x)





# prostie(copie)
# print len(labels_start), len(labels_end)


