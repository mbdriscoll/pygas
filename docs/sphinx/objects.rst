Classes and Methods
===========================

.. automodule:: pygas

PyGAS is composed of a small set of core components that implement the PGAS extension to Python, a modest set of convenience functions for higher-level functionality, and a collection of tools commonly used when writing programs using PyGAS.

Core Components
--------------------------

.. autoclass:: pygas.Proxy
   :members: __init__, __getattr__, __setattr__, __call__, resolve

   .. data:: owner

       The id of the thread where the original object resides.


.. autofunction:: pygas.share


Extended Components
--------------------------

.. autodata:: pygas.THREADS

.. autodata:: pygas.MYTHREAD

.. autofunction:: pygas.broadcast



Miscellaneous Tools
--------------------------

.. autoclass:: pygas.SplitTimer
   :members:
