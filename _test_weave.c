/* Copyright (c) Meta Platforms, Inc. and affiliates. */

#include "ft_utils.h"
#include "ft_weave.h"

static MUTEX_TYPE destructor_mutex;

static int destructor_called_1 = 0;
static int destructor_called_2 = 0;
static int tls_check_1 = 0;
static int tls_check_2 = 0;
static weave_local void* tls_1 = &tls_check_1;
static weave_local void* tls_2 = NULL;

static int sentinel_1 = 0x12345678;
static void* sentinel_1_ptr = &sentinel_1;

static int sentinel_2 = 0x87654321;
static void* sentinel_2_ptr = &sentinel_2;

static void tls_1_write(void *value) {
  fprintf(
    stderr,
    "thread %lu changing tls_1 at %p from %p to %p\n",
#ifdef _WIN32
    (unsigned long) GetCurrentThreadId(),
#else
    (unsigned long) pthread_self(),
#endif
    &tls_1,
    tls_1,
    value
  );
  tls_1 = value;
}

static PyObject* test_reset(
    PyObject* Py_UNUSED(self),
    PyObject* Py_UNUSED(args)) {
  Py_BEGIN_ALLOW_THREADS;
  MUTEX_LOCK(destructor_mutex);
  destructor_called_1 = 0;
  destructor_called_2 = 0;
  tls_check_1 = 0;
  tls_check_2 = 0;
  tls_1_write((void*)0xDEADBEEF);
  tls_2 = NULL;
  MUTEX_UNLOCK(destructor_mutex);
  Py_END_ALLOW_THREADS;
  Py_RETURN_NONE;
}

static void test_destructor_add_1(void* addr) {
  fprintf(
    stderr,
    "thread %lu looking at tls_1 at %p with value %p compared to %p\n",
#ifdef _WIN32
    (unsigned long) GetCurrentThreadId(),
#else
    (unsigned long) pthread_self(),
#endif
    &tls_1,
    tls_1,
    addr
  );

  MUTEX_LOCK(destructor_mutex);
  if (addr == tls_1) {
    destructor_called_1 += 1;
  } else {
    tls_check_1 = 1;
  }
  MUTEX_UNLOCK(destructor_mutex);
}

static void test_destructor_reset_1(void* addr) {
  MUTEX_LOCK(destructor_mutex);
  if (addr == tls_1) {
    destructor_called_1 = 100;
  } else {
    tls_check_1 = 1;
  }
  MUTEX_UNLOCK(destructor_mutex);
}

static void test_destructor_add_2(void* addr) {
  MUTEX_LOCK(destructor_mutex);
  if (addr == tls_2) {
    destructor_called_2 += 1;
  } else {
    tls_check_2 = 1;
  }
  MUTEX_UNLOCK(destructor_mutex);
}

static PyObject* test_weave_get_destructor_called_1(
    PyObject* Py_UNUSED(self),
    PyObject* Py_UNUSED(args)) {
  int c1;
  Py_BEGIN_ALLOW_THREADS;
  MUTEX_LOCK(destructor_mutex);
  if (tls_check_1) {
    MUTEX_UNLOCK(destructor_mutex);
    Py_BLOCK_THREADS;
    PyErr_SetString(
        PyExc_RuntimeError,
        "Incorrect call back address for test_destructor_1");
    return NULL;
  }
  c1 = destructor_called_1;
  MUTEX_UNLOCK(destructor_mutex);
  Py_END_ALLOW_THREADS;
  return PyLong_FromLong(c1);
}

static PyObject* test_weave_get_destructor_called_2(
    PyObject* Py_UNUSED(self),
    PyObject* Py_UNUSED(args)) {
  int c2;
  Py_BEGIN_ALLOW_THREADS;
  MUTEX_LOCK(destructor_mutex);
  if (tls_check_2) {
    MUTEX_UNLOCK(destructor_mutex);
    Py_BLOCK_THREADS;
    PyErr_SetString(
        PyExc_RuntimeError,
        "Incorrect call back address for test_destructor_2");
    return NULL;
  }
  c2 = destructor_called_2;
  MUTEX_UNLOCK(destructor_mutex);
  Py_END_ALLOW_THREADS;
  return PyLong_FromLong(c2);
}

static PyObject* test_weave_register_destructor_1(
    PyObject* Py_UNUSED(self),
    PyObject* arg) {
  tls_1_write(sentinel_1_ptr);

  int ret =
      _py_register_wvls_destructor(&sentinel_1_ptr, &test_destructor_add_1);

  if (ret != 0) {
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyObject* test_weave_register_destructor_2(
    PyObject* Py_UNUSED(self),
    PyObject* Py_UNUSED(args)) {
  int ret =
      _py_register_wvls_destructor(&sentinel_2_ptr, &test_destructor_add_2);

  if (ret != 0) {
    return NULL;
  }

  tls_2 = sentinel_2_ptr;
  Py_RETURN_NONE;
}

static PyObject* test_weave_register_destructor_reset_1(
    PyObject* Py_UNUSED(self),
    PyObject* Py_UNUSED(args)) {
  tls_1_write(sentinel_1_ptr);

  int ret =
      _py_register_wvls_destructor(&sentinel_1_ptr, &test_destructor_reset_1);

  if (ret != 0) {
    return NULL;
  }

  Py_RETURN_NONE;
}

static PyObject* test_weave_unregister_destructor_1(
    PyObject* Py_UNUSED(self),
    PyObject* Py_UNUSED(args)) {
  tls_1_write(NULL);

  int unreg = 0;
  int ret = _py_unregister_wvls_destructor(&sentinel_1_ptr, &unreg);
  if (ret != 0) {
    return NULL;
  }

  return PyLong_FromLong(unreg);
}

static PyObject* test_weave_unregister_destructor_2(
    PyObject* Py_UNUSED(self),
    PyObject* Py_UNUSED(args)) {
  int unreg = 0;
  int ret = _py_unregister_wvls_destructor(&sentinel_2_ptr, &unreg);
  if (ret != 0) {
    return NULL;
  }

  tls_2 = NULL;
  return PyLong_FromLong(unreg);
}

static PyMethodDef test_weave_module_methods[] = {
    {"reset", test_reset, METH_VARARGS, "Reset the destructor test values."},
    {"register_destructor_1",
     test_weave_register_destructor_1,
     METH_NOARGS,
     "Register a reset destructor for a tls 1"},
    {"register_destructor_reset_1",
     test_weave_register_destructor_reset_1,
     METH_NOARGS,
     "Register a destructor for a tls 1"},
    {"unregister_destructor_1",
     test_weave_unregister_destructor_1,
     METH_NOARGS,
     "Unregister a destructor for a tls 1"},
    {"register_destructor_2",
     test_weave_register_destructor_2,
     METH_NOARGS,
     "Register a destructor for a tls 2"},
    {"unregister_destructor_2",
     test_weave_unregister_destructor_2,
     METH_NOARGS,
     "Unregister a destructor for a tls 2"},
    {"get_destructor_called_1",
     test_weave_get_destructor_called_1,
     METH_NOARGS,
     "Get the value of destructor_called_1"},
    {"get_destructor_called_2",
     test_weave_get_destructor_called_2,
     METH_NOARGS,
     "Get the value of destructor_called_2"},
    {NULL, NULL, 0, NULL}};

static int exec_test_weave_module(PyObject* module) {
  return MUTEX_INIT(destructor_mutex);
}

static struct PyModuleDef_Slot test_weave_module_slots[] = {
    {Py_mod_exec, exec_test_weave_module},
    _PY_NOGIL_MODULE_SLOT // NOLINT
    {0, NULL} /* sentinel */
};

static PyModuleDef test_weave_module = {
    PyModuleDef_HEAD_INIT,
    "_test_weave",
    "The native part of testing weave for managing thread based functionality.",
    0,
    test_weave_module_methods,
    test_weave_module_slots,
    NULL,
    NULL,
    NULL};

PyMODINIT_FUNC PyInit__test_weave(void) {
  return PyModuleDef_Init(&test_weave_module);
}
