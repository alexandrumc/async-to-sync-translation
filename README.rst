===============
NUME PROIECT
===============

:Authors:


.. contents::
    :backlinks: none

.. sectnum::


Introduction
============

What is NUME PROIECT?
------------------

**NUME PROIECT** is a tool that, given a communication-closed asynchronous protocol, soundly computes its round-based synchronous counterpart. 

Communication-closed protocol: the reception of a message is effectful only if its timestamp is equal to or greater than the local time of the receiver. In other words, stale messages are discarded.


What is it good for?
--------------------

Fault-tolerant distributed systems are implemented over asynchronous networks, so that they use algorithms for asynchronous models with faults. Due to asynchronous communication and the occurrence of faults (e.g., process crashes or the network dropping messages) the implementations are hard to understand and analyze. In contrast, synchronous computation models simplify design and reasoning. A synchronous version of your asynchronous protocol can be computed using **NUME PROIECT**.

What class of algorithms can this tool translate?
------------------------------------------

**NUME PROIECT** can compute the synchronous counterpart of known consensus and leader election protocols, such as, Paxos, and Chandra and Toueg’s consensus.

Some other examples of protocols that we translated can be found here: `examples <https://github.com/alexandrumc/async-to-sync-translation/tree/master/examples/c_files/TODO>`_.

Our output for the aforementioned list of examples can be found here: `outputs <https://github.com/alexandrumc/async-to-sync-translation/tree/master/examples/c_files/Sync>`_.

-----------------------------------

Using
==========

Prerequisites
-------------

* Python 2.7::

    sudo apt-get install python2.7

* Linux platform
* `Verifast Linux/x64 <http://82076e0e62875f063ae8-929808a701855dfb71539d0a4342d4be.r54.cf5.rackcdn.com/verifast-nightly.tar.gz>`_.


Steps
-----------------------------------

1. Clone project
2. Install Python
3. Download Verifast



------------------------------------------

C code almost always ``#include``\s various header files from the standard C
library, like ``stdio.h``. While (with some effort) **pycparser** can be made to
parse the standard headers from any C compiler, it's much simpler to use the
provided "fake" standard  includes in ``utils/fake_libc_include``. These are
standard C header files that contain only the bare necessities to allow valid
parsing of the files that use them. As a bonus, since they're minimal, it can
significantly improve the performance of parsing large C files.

The key point to understand here is that **pycparser** doesn't really care about
the semantics of types. It only needs to know whether some token encountered in
the source is a previously defined type. This is essential in order to be able
to parse C correctly.

See `this blog post
<http://eli.thegreenplace.net/2015/on-parsing-c-type-declarations-and-fake-headers>`_
for more details.

Basic usage
-----------

Take a look at the |examples|_ directory of the distribution for a few examples
of using **pycparser**. These should be enough to get you started. Please note
that most realistic C code samples would require running the C preprocessor
before passing the code to **pycparser**; see the previous sections for more
details.

.. |examples| replace:: ``examples``
.. _examples: examples


Advanced usage
--------------

The public interface of **pycparser** is well documented with comments in
``pycparser/c_parser.py``. For a detailed overview of the various AST nodes
created by the parser, see ``pycparser/_c_ast.cfg``.

There's also a `FAQ available here <https://github.com/eliben/pycparser/wiki/FAQ>`_.
In any case, you can always drop me an `email <eliben@gmail.com>`_ for help.


Package contents
================

Once you unzip the ``pycparser`` package, you'll 
the following files and
directories:

README.rst:
  This README file.

LICENSE:
  The pycparser license

setup.py:
  Installation script

examples/:
  A directory with some examples of using **pycparser**

pycparser/:
  The **pycparser** module source code.

tests/:
  Unit tests.

utils/fake_libc_include:
  Minimal standard C library include files that should allow to parse any C code.

utils/internal/:
  Internal utilities for my own use. You probably don't need them.


Contributors
============

Some people have contributed to **pycparser** by opening issues on bugs they've
found and/or submitting patches. The list of contributors is in the CONTRIBUTORS
file in the source distribution. After **pycparser** moved to Github I stopped
updating this list because Github does a much better job at tracking
contributions.


CI Status
=========

**pycparser** has automatic testing enabled through the convenient
`Travis CI project <https://travis-ci.org>`_. Here is the latest build status:

.. image:: https://travis-ci.org/eliben/pycparser.png?branch=master
  :align: center
  :target: https://travis-ci.org/eliben/pycparser

AppVeyor also helps run tests on Windows:

.. image:: https://ci.appveyor.com/api/projects/status/wrup68o5y8nuk1i9?svg=true
  :align: center
  :target: https://ci.appveyor.com/project/eliben/pycparser/
