from take_lab import take_code_from_file, get_extern_while_body_from_func, get_context
from pycparser import parse_file

from modify_whiles import *
import os
from compute_paths import get_label, prune_tree, get_extern_while_body, duplicate_element
generator = c_generator.CGenerator()



ast = parse_file(filename="../examples/c_files/verified_copy/Leader-election-Zab_modified.c", use_cpp=False)
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

# print generator.visit(ast)
# label1_list = get_label(ast, "round", "FOURTH_ROUND")
# label2_list = get_label(ast, "round", "AUX_ROUND")
#
# for source in label1_list:
#     for dest in label2_list:
#         print "a"
#         aux_ast = duplicate_element(ast)
#         dest_list = []
#         source_list = []
#         prune_tree(get_extern_while_body(aux_ast), source, dest, dest_list, source_list)
#         if dest_list and source_list:
#             print generator.visit(get_extern_while_body(aux_ast))
#             context = []
#             get_context(get_extern_while_body(aux_ast), context)
#             for elem in context:
#                 print generator.visit(elem.cond)



take_code_from_file(ast,"../examples/c_files/verified_copy/Leader-election-Zab_modified.c",'round')
#print ast
# copie = copy.deepcopy(x)





# prostie(copie)
# print len(labels_start), len(labels_end)


