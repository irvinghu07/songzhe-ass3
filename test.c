#include "unity/src/unity.h"
#include "mem.h"
#include <string.h>

int m_error;

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_no_space(void)
{
    TEST_ASSERT_NULL(mem_alloc((1 << 12) + 1, M_BESTFIT));
    TEST_ASSERT_EQUAL_INT(m_error, E_NO_SPACE);
}

void test_mem_init_twice(void)
{
    TEST_ASSERT_EQUAL_INT(mem_init(1 << 12), -1);
    TEST_ASSERT_EQUAL_INT(m_error, E_BAD_ARGS);
}

void test_allocation(void)
{
    unsigned long *node1 = mem_alloc(sizeof(unsigned long), 0);
    TEST_ASSERT_NOT_NULL(node1);
    memset(node1, 0, sizeof(unsigned long));
    *node1 = 1;
    TEST_ASSERT_EQUAL_INT(*node1, 1);
    TEST_ASSERT_EQUAL_INT(mem_free(node1), 0);
}

void test_linked_list(void)
{
    unsigned long *node1 = mem_alloc(sizeof(unsigned long), 0);
    TEST_ASSERT_NOT_NULL(node1);
    memset(node1, 0, sizeof(unsigned long));
    *node1 = 1;
    mem_dump();
    // array of 10 unsigned long
    unsigned long *node2 = mem_alloc(sizeof(unsigned long) * 10, 0);
    TEST_ASSERT_NOT_NULL(node2);
    memset(node2, 0, sizeof(unsigned long) * 10);
    *node2 = 2;
    mem_dump();
    unsigned long *node3 = mem_alloc(sizeof(unsigned long), 0);
    TEST_ASSERT_NOT_NULL(node3);
    memset(node3, 0, sizeof(unsigned long));
    *node3 = 3;
    mem_dump();
    unsigned long *node4 = mem_alloc(sizeof(unsigned long), 0);
    TEST_ASSERT_NOT_NULL(node1);
    memset(node4, 0, sizeof(unsigned long));
    *node4 = 4;
    mem_dump();
    mem_free(node1);
    mem_free(node2);
    mem_free(node3);
    mem_free(node4);
}

void test_coalescing(void)
{
    unsigned long *node1 = mem_alloc(sizeof(unsigned long), 0);
    TEST_ASSERT_NOT_NULL(node1);
    memset(node1, 0, sizeof(unsigned long));
    *node1 = 1;
    // array of 10 unsigned long
    unsigned long *node2 = mem_alloc(sizeof(unsigned long) * 10, 0);
    TEST_ASSERT_NOT_NULL(node2);
    memset(node2, 0, sizeof(unsigned long) * 10);
    *node2 = 2;
    unsigned long *node3 = mem_alloc(sizeof(unsigned long), 0);
    TEST_ASSERT_NOT_NULL(node3);
    memset(node3, 0, sizeof(unsigned long));
    *node3 = 3;
    unsigned long *node4 = mem_alloc(sizeof(unsigned long), 0);
    TEST_ASSERT_NOT_NULL(node1);
    memset(node4, 0, sizeof(unsigned long));
    *node4 = 4;
    mem_dump();
    mem_free(node2);
    mem_dump();
    mem_free(node3);
    mem_dump();
    mem_free(node1);
    mem_dump();
    mem_free(node4);
    mem_dump();
}

int main(void)
{
    UNITY_BEGIN();
    mem_init(1 << 12);
    RUN_TEST(test_no_space);
    RUN_TEST(test_mem_init_twice);
    RUN_TEST(test_allocation);
    RUN_TEST(test_linked_list);
    RUN_TEST(test_coalescing);
    return UNITY_END();
}

// int main()
// {
//     mem_init(4096);
//     unsigned long *node1 = mem_alloc(sizeof(unsigned long), 0);
//     memset(node1, 0, sizeof(unsigned long));
//     *node1 = 1;
//     mem_dump();
//     // array of 10 unsigned long
//     unsigned long *node2 = mem_alloc(sizeof(unsigned long) * 10, 0);
//     memset(node2, 0, sizeof(unsigned long) * 10);
//     *node2 = 2;
//     mem_dump();
//     unsigned long *node3 = mem_alloc(sizeof(unsigned long), 0);
//     memset(node3, 0, sizeof(unsigned long));
//     *node3 = 3;
//     mem_dump();
//     mem_free(node2);
//     mem_dump();
//     unsigned long *node4 = mem_alloc(sizeof(unsigned long), 0);
//     memset(node4, 0, sizeof(unsigned long));
//     *node4 = 4;
//     mem_dump();
//     printf("Node 1: %lu\nNode 2: %lu\nNode 3: %lu\nNode 4: %lu\n", *node1, *node2, *node3, *node4);
//     return 0;
// }
