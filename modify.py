import copy

from take_lab import *

# from cod_alex import show_func_calls
ast = parse_file(filename="examples/c_files/funky.c", use_cpp=False)
print ast.coord
# ast.show()
# ast.show()
generator = c_generator.CGenerator()
# print(generator.visit(ast))
generator.visit(ast)
# main_function = ast.ext[1]

extern_while_body = None

x = get_extern_while_body(ast)

whiles_to_if(x)


print generator.visit(ast)




copie = copy.deepcopy(ast)

labels = get_labels("examples/c_files/funky.c", "lab")

print labels

trees_dict, paths_dict = get_paths_trees(copie, labels, "lab")
#
# print len(paths_dict), len(trees_dict)



take_code_from_file(copie, "examples/c_files/funky.c", "lab")

trees = trees_dict[labels[0]]
paths = paths_dict[labels[0]]

# print generator.visit(trees[0])


#
#
# to_add_in_path = add_assign_in_tree(get_extern_while_body(tree),path,[])
#
#
#
# add_assign_in_path(path, to_add_in_path)
# print generate_c_code_from_paths(path,tree)


# add_ghost_assign(trees_dict,paths_dict,labels)