#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <iostream>
#include <cassert>
#include "koopa.h"

extern int raw_cnt;
std::string reg_name[16] = {"t0", "t1", "t2", "t3", "t4", "t5", "t6", "x0",
                            "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
std::unordered_map<koopa_raw_value_t, std::pair<int, int>> reg_table;
std::vector<std::string> globl_vars;

std::string get_name(const char* s) {
    std::string str = "";
    int i = 1;
    while (s[i] != 0) {
        str += s[i];
        ++i;
    }
    return str;
}

void visit(koopa_raw_value_t &value, std::string reg, std::string &str) {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        str += "\tli " + reg + ", " + std::to_string(value->kind.data.integer.value) + "\n";
        return;
    }
    if (reg_table.find(value) != reg_table.end()) {
        str += "\tlw " + reg + ", " + std::to_string(reg_table[value].first) + "(sp)\n";
        return;
    }
    auto it = globl_vars.begin();
    for (; it != globl_vars.end() && get_name(value->name) != *it; ++it);
    if (it != globl_vars.end()) {
        str += "\tlw " + reg + ", " + get_name(value->name) + "\n";
        return;
    }
}

void get_stack_size(koopa_raw_function_t &func, int &stack_size, int &S, int &R, int &A) {
    S = func->params.len * 4;

    for (size_t j = 0; j < func->bbs.len; ++j) {
        assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[j];
        
        for (size_t k = 0; k < bb->insts.len; ++k) {
            koopa_raw_value_t value = (koopa_raw_value_t) bb->insts.buffer[k];
            
            switch (value->kind.tag) {
                case KOOPA_RVT_BINARY: {
                    if (reg_table.find(value) == reg_table.end()) {
                        reg_table[value] = std::pair<int, int>(1, 0);
                        S += 4;
                    }
                    break;
                }
                case KOOPA_RVT_LOAD: {
                    if (reg_table.find(value) == reg_table.end()) {
                        reg_table[value] = std::pair<int, int>(1, 0);
                        S += 4;
                    }
                    break;
                }
                case KOOPA_RVT_STORE: {
                    if (reg_table.find(value->kind.data.store.dest) == reg_table.end()) {
                        reg_table[value->kind.data.store.dest] = std::pair<int, int>(1, 0);
                        S += 4;
                    }
                    break;
                }
                case KOOPA_RVT_CALL: {
                    R = 4;
                    koopa_raw_slice_t args = value->kind.data.call.args;
                    A = A >= ((int32_t)args.len - 8) ? A : ((int32_t)args.len - 8);

                    if (value->ty->tag != KOOPA_RTT_UNIT) {
                        if (reg_table.find(value) == reg_table.end()) {
                            reg_table[value] = std::pair<int, int>(1, 0);
                            S += 4;
                        }
                    }

                    break;
                }
                default: break;
            }
        }
    }
    A *= 4;
    stack_size = S + R + A;
    stack_size = (stack_size + 15) / 16 * 16;
}

void raw2riscv(koopa_raw_program_t &raw, std::string &str) {

    globl_vars.clear();

    for (size_t i = 0; i < raw.values.len; ++i) {
        assert(raw.values.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t value = (koopa_raw_value_t) raw.values.buffer[i];
        globl_vars.push_back(get_name(value->name));
        
        if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
            koopa_raw_value_t init = value->kind.data.global_alloc.init;

            str += "\t.data\n";
            str += "\t.globl " + get_name(value->name) + "\n" + get_name(value->name) + ":\n";

            switch (init->kind.tag) {
                case KOOPA_RVT_INTEGER: {
                    str += "\t.word " + std::to_string(init->kind.data.integer.value) + "\n";
                    break;
                }
                case KOOPA_RVT_ZERO_INIT: {
                    str += "\t.zero 4\n";
                    break;
                }
                default: break;
            }
        }
        str += "\n";
    }

    for (size_t i = 0; i < raw.funcs.len; ++i) {
        assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];
        if (func->bbs.len == 0) continue;

        str += "\t.text\n";
        str += "\t.globl " + get_name(func->name) + "\n" + get_name(func->name) + ":\n";
    
        int stack_size = 0, S = 0, R = 0, A = 0;
        get_stack_size(func, stack_size, S, R, A);
        reg_table.clear();
        int offset = A;

        if (stack_size) str += "\taddi sp, sp, -" + std::to_string(stack_size) + "\n";
        if (R) str += "\tsw ra, " + std::to_string(stack_size - R) + "(sp)\n";
        
        for (size_t j = 0; j < func->params.len; ++j) {
            assert(func->params.kind == KOOPA_RSIK_VALUE);
            koopa_raw_value_t param = (koopa_raw_value_t) func->params.buffer[j];
            if (j < 8) reg_table[param] = std::pair<int, int>(offset, 8 + j);
            else reg_table[param] = std::pair<int, int>(stack_size + 4 * (j - 8), 8 + j);
        }

        for (size_t j = 0; j < func->bbs.len; ++j) {
            assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);

            koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[j];
            if (j != 0) str += "\n" + get_name(bb->name) + ":\n";

            for (size_t k = 0; k < bb->insts.len; ++k) {
                koopa_raw_value_t value = (koopa_raw_value_t) bb->insts.buffer[k];

                switch (value->kind.tag) {
                    case KOOPA_RVT_RETURN: {
                        koopa_raw_value_t ret_value = value->kind.data.ret.value;
                        if (ret_value) visit(ret_value, "a0", str);
                        if (R) str += "\tlw ra, " + std::to_string(stack_size - R) + "(sp)\n";
                        if (stack_size) str += "\taddi sp, sp, " + std::to_string(stack_size) + "\n";
                        str += "\tret\n";
                        break;
                        }
                    case KOOPA_RVT_BINARY: {
                        koopa_raw_binary_op_t op = value->kind.data.binary.op;
                        koopa_raw_value_t lhs = value->kind.data.binary.lhs;
                        koopa_raw_value_t rhs = value->kind.data.binary.rhs;
                        visit(lhs, "t0", str);
                        visit(rhs, "t1", str);
                        switch (op) {
                            case KOOPA_RBO_ADD: {
                                str += "\tadd t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_SUB: {
                                str += "\tsub t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_MUL: {
                                str += "\tmul t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_DIV: {
                                str += "\tdiv t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_MOD: {
                                str += "\trem t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_EQ: {
                                str += "\tsub t2, t0, t1\n";
                                str += "\tseqz t3, t2\n";
                                str += "\tsw t3, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_NOT_EQ: {
                                str += "\tsub t2, t0, t1\n";
                                str += "\tsnez t3, t2\n";
                                str += "\tsw t3, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_GE: {
                                str += "\tslt t2, t0, t1\n";
                                str += "\tseqz t3, t2\n";
                                str += "\tsw t3, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_GT: {
                                str += "\tslt t2, t1, t0\n";
                                str += "\tsw t2, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_LE: {
                                str += "\tslt t2, t1, t0\n";
                                str += "\tseqz t3, t2\n";
                                str += "\tsw t3, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_LT: {
                                str += "\tslt t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_AND: {
                                str += "\tand t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;                                
                                break;
                            }
                            case KOOPA_RBO_OR: {
                                str += "\tor t2, t0, t1\n";
                                str += "\tsw t2, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;                                
                                break;
                            } 
                            default: break;                       
                        }
                        break;
                    }
                    case KOOPA_RVT_LOAD: {
                        auto it = globl_vars.begin();
                        for (; it != globl_vars.end() && get_name(value->kind.data.load.src->name) != *it; ++it);
                        if (it != globl_vars.end()) {
                            str += "\tlw t0, " + get_name(value->kind.data.load.src->name) + "\n";
                        }
                        if (reg_table.find(value->kind.data.load.src) != reg_table.end()) {
                            str += "\tlw t0, " + std::to_string(reg_table[value->kind.data.load.src].first) + "(sp)\n";
                        }
                        str += "\tsw t0, " + std::to_string(offset) + "(sp)\n";
                        reg_table[value] = std::pair<int, int>(offset, 0);
                        offset += 4;
                        break;
                    }
                    case KOOPA_RVT_STORE: {
                        if (value->kind.data.store.value->kind.tag == KOOPA_RVT_INTEGER) {
                            str += "\tli t0, " + std::to_string(value->kind.data.store.value->kind.data.integer.value) + "\n";
                        }
                        else {
                            if (reg_table.find(value->kind.data.store.value) != reg_table.end() && (reg_table[value->kind.data.store.value].second == 0 || reg_table[value->kind.data.store.value].second > 15)) {
                                str += "\tlw t0, " + std::to_string(reg_table[value->kind.data.store.value].first) + "(sp)\n";
                                reg_table[value->kind.data.store.value].second = 0;
                            }
                            else {
                                auto it = globl_vars.begin();    
                                for (; it != globl_vars.end() && get_name(value->kind.data.store.value->name) != *it; ++it);
                                if (it == globl_vars.end()) {
                                }
                                else str += "\tlw t0, " + get_name(value->kind.data.store.value->name) + "\n";
                            }
                        }

                        auto it = globl_vars.begin();
                        for (; it != globl_vars.end() && get_name(value->kind.data.store.dest->name) != *it; ++it);
                        if (it == globl_vars.end()) {
                            if (reg_table.find(value->kind.data.store.dest) != reg_table.end()) {
                                str += "\tsw " + reg_name[reg_table[value->kind.data.store.value].second] + ", " + std::to_string(reg_table[value->kind.data.store.dest].first) + "(sp)\n";
                            }
                            else {
                                str += "\tsw " + reg_name[reg_table[value->kind.data.store.value].second] + ", " + std::to_string(offset) + "(sp)\n";
                                reg_table[value->kind.data.store.dest] = std::pair<int, int>(offset, 0);
                                offset += 4;
                            }
                        }
                        else {
                            str += "\tla t1, " + get_name(value->kind.data.store.dest->name) + "\n";
                            str += "\tsw " + reg_name[reg_table[value->kind.data.store.value].second] + ", 0(t1)\n";
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
                                str += "\tlw t0, " + std::to_string(reg_table[cond].first) + "(sp)\n";
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
                    case KOOPA_RVT_CALL: {
                        koopa_raw_call_t call = value->kind.data.call;

                        for (size_t l = 0; l < call.args.len; ++l) {
                            assert(call.args.kind == KOOPA_RSIK_VALUE);
                            koopa_raw_value_t arg = (koopa_raw_value_t) call.args.buffer[l];
                            if (l < 8) visit(arg, reg_name[l + 8], str);
                            else {
                                visit(arg, "t0", str);
                                str += "\tsw t0, " + std::to_string(l * 4 - 32) + "(sp)\n";
                            }
                        }

                        str += "\tcall " + get_name(call.callee->name) + "\n";

                        if (value->ty->tag != KOOPA_RTT_UNIT) {
                            if (reg_table.find(value) != reg_table.end()) {
                                str += "\tsw a0, " + std::to_string(reg_table[value].first) + "(sp)\n";
                            }
                            else {
                                str += "\tsw a0, " + std::to_string(offset) + "(sp)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                            }
                        }
                    }
                    default: break;
                }
            }
        }
        str += "\n";
    }
}