from utils import get_label, get_epochs_assigns, get_send_usage, get_recv_whiles
from pycparser import c_generator
from pycparser.plyparser import Coord

generator = c_generator.CGenerator()


#get_send_usage intoarce toate apelurile functiei send


def create_tag_strict_leq_assert(old_phase, old_round, phase, round):
    aux = '//@ assert tag_strict_leq(%s,%s,%s,%s)' % (old_phase, old_round, phase, round)
    return aux


def create_tag_leq_assert(old_phase, old_round, phase, round):
    aux = '//@ assert tag_leq(%s,%s,%s,%s)' % (old_phase, old_round, phase, round)
    return aux


def create_mbox_tag_eq_assert(phase, round, mbox_name):
    aux = '//@ assert mbox_tag_EQ(%s,%s,%s)' % (phase, round, mbox_name)
    return aux

def create_mbox_tag_geq_assert(phase, round, mbox_name):
    aux = '//@ assert mbox_tag_GEQ(%s,%s,%s)' % (phase, round, mbox_name)
    return aux

def create_max_tag_mbox_assert(epoch, round, mbox):
    aux ='//@ assert max_tag_mbox(%s,%s,%s)' %(epoch, round, mbox)
    return aux


def identify_round_epoch_assigns(ast, label_name, epoch_name, labels):
    """
    identifies all round an epoch variables assignments(at least I hope so)
    :param ast:
    :param label_name:
    :param epoch_name:
    :param labels:
    :return:
    """
    labs = []
    for label in labels:
        aux = get_label(ast, label_name, label)
        labs = labs + aux

    epoch_assigns = get_epochs_assigns(ast, epoch_name)

    for a in labs:
        print generator.visit(a), a.coord
    for a in epoch_assigns:
        print generator.visit(a), a.coord

def identify_send_functions(ast):
    """
    identifies all usages of send function
    :param ast:
    :return:
    """
    sends = get_send_usage(ast)
    for send in sends:
        print send.coord
