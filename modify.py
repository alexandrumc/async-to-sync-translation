import copy

from take_lab import take_code_from_file, get_extern_while_body_from_func
from pycparser import parse_file

from parse_test import get_label, prune_tree
from modify_whiles import *
generator = c_generator.CGenerator()




#
ast = parse_file(filename="examples/c_files/tpc_AMIT_modificat.c", use_cpp=False)
extern_while_body = None

x = get_extern_while_body_from_func(ast, "main")
conditii = []
whiles_to_if(x, conditii)

identify_recv_exits(x, conditii)
remove_mbox(x)
print generator.visit(x)
# test = get_labels_order("examples/c_files/broadcast.c",'round')
# print test
# print generator.visit(x)

# take_code_from_file(ast,"examples/c_files/tpc_AMIT_modificat.c",'round')
# copie = copy.deepcopy(x)

# labels_start = get_label(copie, 'lab', 'THIRD_ROUND')
# labels_end = get_label(copie, 'lab', 'FOURTH_ROUND')
# prune_tree(copie,labels_start[0],labels_end[0],[],[])
# print generator.visit(copie)



# prostie(copie)
# print len(labels_start), len(labels_end)


