// RUN: %clangxx_asan -O0 %p/dll_host.cc -Fe%t
// RUN: %clangxx_asan -LD -O0 %s -Fe%t.dll
// FIXME: 'cat' is needed due to PR19744.
// RUN: not %run %t %t.dll 2>&1 | cat | FileCheck %s

#include <stdio.h>
#include <string.h>

void call_memcpy(void* (*f)(void *, const void *, size_t),
                 void *a, const void *b, size_t c) {
  f(a, b, c);
}

extern "C" __declspec(dllexport)
int test_function() {
  char buff1[6] = "Hello", buff2[5];

  call_memcpy(&memcpy, buff2, buff1, 5);
  if (buff1[2] != buff2[2])
    return 2;
  printf("Initial test OK\n");
  fflush(0);
// CHECK: Initial test OK

  call_memcpy(&memcpy, buff2, buff1, 6);
// CHECK: AddressSanitizer: stack-buffer-overflow on address [[ADDR:0x[0-9a-f]+]]
// CHECK: WRITE of size 6 at [[ADDR]] thread T0
// CHECK-NEXT:  __asan_memcpy
// CHECK-NEXT:  call_memcpy
// CHECK-NEXT:  test_function {{.*}}dll_intercept_memcpy.cc:[[@LINE-5]]
// CHECK: Address [[ADDR]] is located in stack of thread T0 at offset {{.*}} in frame
// CHECK-NEXT:  test_function {{.*}}dll_intercept_memcpy.cc
// CHECK: 'buff2' <== Memory access at offset {{.*}} overflows this variable
}
