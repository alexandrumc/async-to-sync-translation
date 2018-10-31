from utils import get_label, get_epochs_assigns
from pycparser import c_generator
from pycparser.plyparser import Coord

generator = c_generator.CGenerator()


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
    labs = []
    for label in labels:
        aux = get_label(ast, label_name, label)
        labs = labs + aux

    epoch_assigns = get_epochs_assigns(ast, epoch_name)

    for a in labs:
        print generator.visit(a), a.coord
    for a in epoch_assigns:
        print generator.visit(a), a.coord
