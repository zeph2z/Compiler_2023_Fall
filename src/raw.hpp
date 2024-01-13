#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <unordered_map>
#include <cassert>
#include "koopa.h"

extern int raw_cnt;
std::string reg_name[16] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "x0",
                            "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
std::unordered_map<koopa_raw_value_t, int> reg_table;

std::string get_name(const char* s) {
    std::string str = "";
    int i = 1;
    while (s[i] != 0) {
        str += s[i];
        ++i;
    }
    return str;
}

int get_stack_size(koopa_raw_function_t &func) {
    int stack_size = 0;
    for (size_t j = 0; j < func->bbs.len; ++j) {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[j];
        
        for (size_t k = 0; k < bb->insts.len; ++k) {
            koopa_raw_value_t value = (koopa_raw_value_t) bb->insts.buffer[k];
            if (value->kind.tag == KOOPA_RVT_BLOCK_ARG_REF || value->kind.tag == KOOPA_RVT_LOAD || value->kind.tag == KOOPA_RVT_BINARY) {
                if (reg_table.find(value) == reg_table.end()) {
                    reg_table[value] = stack_size;
                    stack_size += 4;
                }
            }
        }
    }
    stack_size = (stack_size + 15) / 16 * 16;
    return stack_size;
}

void raw2riscv(koopa_raw_program_t &raw, std::string &str) {
    for (size_t i = 0; i < raw.funcs.len; ++i) {
        assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];
        if (func->bbs.len == 0) continue;

        str += "\t.text\n";
        str += "\t.globl " + get_name(func->name) + "\n" + get_name(func->name) + ":\n";
    
        int stack_size = 0;

        str += "\taddi sp, sp, -256\n";

        for (size_t j = 0; j < func->bbs.len; ++j) {
            assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
            koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[j];
            if (j != 0) str += "\n" + get_name(bb->name) + ":\n";

            for (size_t k = 0; k < bb->insts.len; ++k) {
                koopa_raw_value_t value = (koopa_raw_value_t) bb->insts.buffer[k];

                switch (value->kind.tag) {
                    case KOOPA_RVT_RETURN: {
                        koopa_raw_value_t ret_value = value->kind.data.ret.value;
                        if (ret_value->kind.tag == KOOPA_RVT_INTEGER) {
                            int32_t int_val = ret_value->kind.data.integer.value;
                            str += "\tli a0, " + std::to_string(int_val) + "\n";
                        }
                        else {
                            if (reg_table.find(ret_value) != reg_table.end())
                                str += "\tlw a0, " + std::to_string(reg_table[ret_value]) + "(sp)\n";
                        }
                        str += "\taddi sp, sp, 256\n";
                        str += "\tret";
                        break;
                        }
                    case KOOPA_RVT_BINARY: {
                        koopa_raw_binary_op_t op = value->kind.data.binary.op;
                        koopa_raw_value_t lhs = value->kind.data.binary.lhs;
                        koopa_raw_value_t rhs = value->kind.data.binary.rhs;
                        if (lhs->kind.tag == KOOPA_RVT_INTEGER) {
                            str += "\tli t0, " + std::to_string(lhs->kind.data.integer.value) + "\n";                            
                        }
                        else {
                            if (reg_table.find(lhs) != reg_table.end())
                                str += "\tlw t0, " + std::to_string(reg_table[lhs]) + "(sp)\n";
                        }
                        if (rhs->kind.tag == KOOPA_RVT_INTEGER) {
                            str += "\tli t1, " + std::to_string(rhs->kind.data.integer.value) + "\n";
                        }
                        else {
                            if (reg_table.find(rhs) != reg_table.end())
                                str += "\tlw t1, " + std::to_string(reg_table[rhs]) + "(sp)\n";
                        }
                        switch (op) {
                            case KOOPA_RBO_ADD: {
                                str += "\tadd t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_SUB: {
                                str += "\tsub t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_MUL: {
                                str += "\tmul t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_DIV: {
                                str += "\tdiv t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_MOD: {
                                str += "\trem t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_EQ: {
                                str += "\tsub t2, t0, t1\n";
                                str += "\tseqz t3, t2\n";
                                str += "\tsw t3, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_NOT_EQ: {
                                str += "\tsub t2, t0, t1\n";
                                str += "\tsnez t3, t2\n";
                                str += "\tsw t3, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_GE: {
                                str += "\tslt t2, t0, t1\n";
                                str += "\tseqz t3, t2\n";
                                str += "\tsw t3, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_GT: {
                                str += "\tslt t2, t1, t0\n";
                                str += "\tsw t2, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_LE: {
                                str += "\tslt t2, t1, t0\n";
                                str += "\tseqz t3, t2\n";
                                str += "\tsw t3, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_LT: {
                                str += "\tslt t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;
                                break;
                            }
                            case KOOPA_RBO_AND: {
                                str += "\tand t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;                                
                                break;
                            }
                            case KOOPA_RBO_OR: {
                                str += "\tor t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(stack_size) + "(sp)\n";
                                reg_table[value] = stack_size;
                                stack_size += 4;                                
                                break;
                            }                        
                        }
                        break;
                    }
                    case KOOPA_RVT_LOAD: {
                        if (reg_table.find(value->kind.data.load.src) != reg_table.end()) {
                            str += "\tlw t0, " + std::to_string(reg_table[value->kind.data.load.src]) + "(sp)\n";
                        }
                        str += "\tsw t0, " + std::to_string(stack_size) + "(sp)\n";
                        reg_table[value] = stack_size;
                        stack_size += 4;
                        break;
                    }
                    case KOOPA_RVT_STORE: {
                        if (value->kind.data.store.value->kind.tag == KOOPA_RVT_INTEGER) {
                            str += "\tli t0, " + std::to_string(value->kind.data.store.value->kind.data.integer.value) + "\n";
                        }
                        else {
                            if (reg_table.find(value->kind.data.store.value) != reg_table.end()) {
                                str += "\tlw t0, " + std::to_string(reg_table[value->kind.data.store.value]) + "(sp)\n";
                            }
                        }
                        if (reg_table.find(value->kind.data.store.dest) != reg_table.end()) {
                            str += "\tsw t0, " + std::to_string(reg_table[value->kind.data.store.dest]) + "(sp)\n";
                        }
                        else {
                            str += "\tsw t0, " + std::to_string(stack_size) + "(sp)\n";
                            reg_table[value->kind.data.store.dest] = stack_size;
                            stack_size += 4;
                        }
                        break;
                    }
                    case KOOPA_RVT_BRANCH: {
                        koopa_raw_value_t cond = value->kind.data.branch.cond;
                        const char* true_bb = value->kind.data.branch.true_bb->name;
                        const char* false_bb = value->kind.data.branch.false_bb->name;
                        if (cond->kind.tag == KOOPA_RVT_INTEGER) {
                            int32_t int_val = cond->kind.data.integer.value;
                            str += "\tli t0, " + std::to_string(int_val) + "\n";
                        }
                        else {
                            if (reg_table.find(cond) != reg_table.end())
                                str += "\tlw t0, " + std::to_string(reg_table[cond]) + "(sp)\n";
                        }
                        str += "\tbnez t0, " + get_name(true_bb) + "\n";
                        str += "\tj " + get_name(false_bb) + "\n";
                        break;
                    }
                    case KOOPA_RVT_JUMP: {
                        const char* bb = value->kind.data.jump.target->name;
                        str += "\tj " + get_name(bb) + "\n";
                        break;
                    }
                }
            }
        }
        str += "\n";
    }
}