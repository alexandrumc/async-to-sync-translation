from take_lab import take_code_from_file, get_extern_while_body_from_func, get_context
from pycparser import parse_file

from modify_whiles import *
import sys

ast = parse_file(filename=sys.argv[1], use_cpp=False)
x = get_extern_while_body_from_func(ast, "main")
conditii = []
whiles_to_if(x, conditii)

identify_recv_exits(x, conditii)
remove_mbox(x)
take_code_from_file(ast,sys.argv[1],'round')


