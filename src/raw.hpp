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
std::unordered_map<koopa_raw_value_t, int> ptrs;

std::string get_name(const char* s) {
    std::string str = "";
    int i = 1;
    while (s[i] != 0) {
        str += s[i];
        ++i;
    }
    return str;
}

int value_is_ptr(const koopa_raw_value_t &value) {
    if (ptrs.find(value) != ptrs.end()) return ptrs[value];
    return 0;
}

void visit(koopa_raw_value_t &value, std::string reg, std::string &str) {
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
        str += "\tli " + reg + ", " + std::to_string(value->kind.data.integer.value) + "\n";
        return;
    }
    if (reg_table.find(value) != reg_table.end()) {
        str += "\tli t4, " + std::to_string(reg_table[value].first) + "\n";
        str += "\tadd t4, t4, sp\n";
        for (int z = 0; z < value_is_ptr(value); ++z) {
            str += "\tlw t4, 0(t4)\n";
        }
        str += "\tlw " + reg + ", 0(t4)\n";
        return;
    }
    auto it = globl_vars.begin();
    for (; it != globl_vars.end() && get_name(value->name) != *it; ++it);
    if (it != globl_vars.end()) {
        str += "\tlw " + reg + ", " + get_name(value->name) + "\n";
        return;
    }
}

int get_array_size(const koopa_raw_type_kind *ty) {
    if (ty->tag == KOOPA_RTT_INT32) return 4;
    if (ty->tag == KOOPA_RTT_POINTER) return get_array_size(ty->data.pointer.base);
    if (ty->tag == KOOPA_RTT_ARRAY) return ty->data.array.len * get_array_size(ty->data.array.base);
    return 0;
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
                case KOOPA_RVT_GET_ELEM_PTR: {
                    koopa_raw_value_t src = value->kind.data.get_elem_ptr.src;
                    // src should be a pointer
                    
                    if (reg_table.find(src) == reg_table.end()) {
                        reg_table[src] = std::pair<int, int>(1, 0);
                        S += get_array_size(src->ty);
                    }
                    if (reg_table.find(value) == reg_table.end()) {
                        reg_table[value] = std::pair<int, int>(1, 0);
                        S += 4;
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

void global_init(koopa_raw_value_t &elems, int size, std::string &str) {
    if (elems->ty->tag == KOOPA_RTT_INT32) {
        if (elems->kind.tag == KOOPA_RVT_INTEGER) {
            str += "\t.word " + std::to_string(elems->kind.data.integer.value) + "\n";
        }
        else if (elems->kind.tag == KOOPA_RVT_ZERO_INIT) {
            str += "\t.zero 4\n";
        }
        return;
    }
    
    if (elems->ty->tag == KOOPA_RTT_ARRAY) {
        if (elems->kind.tag == KOOPA_RVT_AGGREGATE) {
            // init should be an array
            koopa_raw_slice_t items = elems->kind.data.aggregate.elems;

            for (int i = 0; i < items.len; ++i) {
                koopa_raw_value_t elem = (koopa_raw_value_t) items.buffer[i];
                std::cout << elem->ty->tag << " " << elem->kind.tag << std::endl;
                global_init(elem, 0, str);
            }
        }
        else if (elems->kind.tag == KOOPA_RVT_ZERO_INIT) {
            str += "\t.zero " + std::to_string(size) + "\n";
        }
    }

}

void addi_sp(int size, bool add, std::string &str) {
    std::string op = add ? "" : "-";
    while (size >= 2047) {
        str += "\taddi sp, sp, " + op + "2047\n";
        size -= 2047;
    }
    if (size) str += "\taddi sp, sp, " + op + std::to_string(size) + "\n";
}

void raw2riscv(koopa_raw_program_t &raw, std::string &str) {

    globl_vars.clear();
    ptrs.clear();

    for (size_t i = 0; i < raw.values.len; ++i) {
        assert(raw.values.kind == KOOPA_RSIK_VALUE);
        koopa_raw_value_t value = (koopa_raw_value_t) raw.values.buffer[i];
        globl_vars.push_back(get_name(value->name));
        
        if (value->ty->tag == KOOPA_RTT_INT32) {
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
        }
        else if (value->ty->tag == KOOPA_RTT_POINTER) {
            str += "\t.data\n";
            str += "\t.globl " + get_name(value->name) + "\n" + get_name(value->name) + ":\n";
            
            koopa_raw_value_t init = value->kind.data.global_alloc.init;
            global_init(init, get_array_size(value->ty), str);
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

        std::cout << stack_size << " " << S << " " << R << " " << A << std::endl;

        addi_sp(stack_size, false, str);
        if (R) {
            str += "\tli t4, " + std::to_string(stack_size - R) + "\n";
            str += "\tadd t4, t4, sp\n";
            str += "\tsw ra, 0(t4)\n";
        }
        
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
                        if (R) {
                            str += "\tli t4, " + std::to_string(stack_size - R) + "\n";
                            str += "\tadd t4, t4, sp\n";
                            str += "\tlw ra, 0(t4)\n";
                        }
                        addi_sp(stack_size, true, str);
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
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t2, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_SUB: {
                                str += "\tsub t2, t0, t1\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t2, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_MUL: {
                                str += "\tmul t2, t0, t1\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t2, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_DIV: {
                                str += "\tdiv t2, t0, t1\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t2, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_MOD: {
                                str += "\trem t2, t0, t1\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t2, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_EQ: {
                                str += "\tsub t2, t0, t1\n";
                                str += "\tseqz t3, t2\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t3, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_NOT_EQ: {
                                str += "\tsub t2, t0, t1\n";
                                str += "\tsnez t3, t2\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t3, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_GE: {
                                str += "\tslt t2, t0, t1\n";
                                str += "\tseqz t3, t2\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t3, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_GT: {
                                str += "\tslt t2, t1, t0\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t2, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_LE: {
                                str += "\tslt t2, t1, t0\n";
                                str += "\tseqz t3, t2\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t3, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_LT: {
                                str += "\tslt t2, t0, t1\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t2, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                                break;
                            }
                            case KOOPA_RBO_AND: {
                                str += "\tand t2, t0, t1\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t2, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;                                
                                break;
                            }
                            case KOOPA_RBO_OR: {
                                str += "\tor t2, t0, t1\n";
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t2, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;                                
                                break;
                            } 
                            default: break;                       
                        }
                        break;
                    }
                    case KOOPA_RVT_LOAD: {
                        if (reg_table.find(value->kind.data.load.src) != reg_table.end()) {
                            str += "\tli t4, " + std::to_string(reg_table[value->kind.data.load.src].first) + "\n";
                            str += "\tadd t4, t4, sp\n";
                            for (int z = 0; z < value_is_ptr(value->kind.data.load.src); ++z) str += "\tlw t4, 0(t4)\n";
                            str += "\tlw t0, 0(t4)\n";
                        }
                        else {
                            auto it = globl_vars.begin();
                            for (; it != globl_vars.end() && get_name(value->kind.data.load.src->name) != *it; ++it);
                            if (it != globl_vars.end()) {
                                str += "\tlw t0, " + get_name(value->kind.data.load.src->name) + "\n";
                            }
                        }
                        str += "\tli t4, " + std::to_string(offset) + "\n";
                        str += "\tadd t4, t4, sp\n";
                        str += "\tsw t0, 0(t4)\n";
                        reg_table[value] = std::pair<int, int>(offset, 0);
                        offset += 4;
                        break;
                    }
                    case KOOPA_RVT_STORE: {
                        if (value->kind.data.store.value->kind.tag == KOOPA_RVT_INTEGER) {
                            str += "\tli t0, " + std::to_string(value->kind.data.store.value->kind.data.integer.value) + "\n";
                            if (reg_table.find(value->kind.data.store.dest) != reg_table.end()) {
                                str += "\tli t4, " + std::to_string(reg_table[value->kind.data.store.dest].first) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                for (int z = 0; z < value_is_ptr(value->kind.data.store.dest); ++z) str += "\tlw t4, 0(t4)\n";
                                str += "\tsw t0, 0(t4)\n";
                            }
                            else {
                                auto it = globl_vars.begin();
                                for (; it != globl_vars.end() && get_name(value->kind.data.store.dest->name) != *it; ++it);
                                if (it != globl_vars.end()) {
                                    str += "\tla t1, " + get_name(value->kind.data.store.dest->name) + "\n";
                                    str += "\tsw t0, 0(t1)\n";
                                }
                                else {
                                    str += "\tli t4, " + std::to_string(offset) + "\n";
                                    str += "\tadd t4, t4, sp\n";
                                    str += "\tsw t0, 0(t4)\n";
                                    reg_table[value->kind.data.store.dest] = std::pair<int, int>(offset, 0);
                                    offset += 4;
                                }
                            }
                            break;
                        }
                        else {
                            if (reg_table.find(value->kind.data.store.value) != reg_table.end() && (reg_table[value->kind.data.store.value].second == 0 || reg_table[value->kind.data.store.value].second > 15)) {
                                str += "\tli t4, " + std::to_string(reg_table[value->kind.data.store.value].first) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                for (int z = 0; z < value_is_ptr(value->kind.data.store.value); ++z) str += "\tlw t4, 0(t4)\n";
                                str += "\tlw t0, 0(t4)\n";
                                reg_table[value->kind.data.store.value].second = 0;
                            }
                            else {
                                auto it = globl_vars.begin();    
                                for (; it != globl_vars.end() && get_name(value->kind.data.store.value->name) != *it; ++it);
                                if (it != globl_vars.end()) {
                                    str += "\tla t1, " + get_name(value->kind.data.store.value->name) + "\n";
                                    str += "\tlw t0, 0(t1)\n";
                                }
                            }
                        }

                        auto it = globl_vars.begin();
                        for (; it != globl_vars.end() && (bool)value->kind.data.store.dest->name && get_name(value->kind.data.store.dest->name) != *it; ++it);
                        if (it == globl_vars.end()) {
                            if (reg_table.find(value->kind.data.store.dest) != reg_table.end()) {
                                str += "\tli t4, " + std::to_string(reg_table[value->kind.data.store.dest].first) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                for (int z = 0; z < value_is_ptr(value->kind.data.store.dest); ++z) str += "\tlw t4, 0(t4)\n";
                                str += "\tsw " + reg_name[reg_table[value->kind.data.store.value].second] + ", 0(t4)\n";
                            }
                            else {
                                str += "\tli t4, " + std::to_string(reg_table[value->kind.data.store.dest].first) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                for (int z = 0; z < value_is_ptr(value->kind.data.store.dest); ++z) str += "\tlw t4, 0(t4)\n";
                                str += "\tsw " + reg_name[reg_table[value->kind.data.store.value].second] + ", 0(t4)\n";
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
                            if (reg_table.find(cond) != reg_table.end()) {
                                str += "\tli t4, " + std::to_string(reg_table[cond].first) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                for (int z = 0; z < value_is_ptr(cond); ++z) str += "\tlw t4, 0(t4)\n";
                                str += "\tlw t0, 0(t4)\n";
                            }
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
                                str += "\tli t4, " + std::to_string(l * 4 - 32) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw t0, 0(t4)\n";
                            }
                        }

                        str += "\tcall " + get_name(call.callee->name) + "\n";

                        if (value->ty->tag != KOOPA_RTT_UNIT) {
                            if (reg_table.find(value) != reg_table.end()) {
                                str += "\tli t4, " + std::to_string(reg_table[value].first) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                for (int z = 0; z < value_is_ptr(value); ++z) str += "\tlw t4, 0(t4)\n";
                                str += "\tsw a0, 0(t4)\n";
                            }
                            else {
                                str += "\tli t4, " + std::to_string(offset) + "\n";
                                str += "\tadd t4, t4, sp\n";
                                str += "\tsw a0, 0(t4)\n";
                                reg_table[value] = std::pair<int, int>(offset, 0);
                                offset += 4;
                            }
                        }
                    }
                    case KOOPA_RVT_GET_ELEM_PTR: {
                        koopa_raw_value_t src = value->kind.data.get_elem_ptr.src;
                        koopa_raw_value_t index = value->kind.data.get_elem_ptr.index;
                        // src should be a pointer
                        size_t len = get_array_size(src->ty);
                        size_t next_len;
                        if (src->ty->data.pointer.base->data.array.base->tag == KOOPA_RTT_ARRAY) next_len = get_array_size(src->ty->data.pointer.base->data.array.base);
                        else next_len = 4;

                        visit(index, "t1", str);
                        str += "\tli t2, " + std::to_string(next_len) + "\n";
                        str += "\tmul t1, t1, t2\n";

                        if (reg_table.find(src) != reg_table.end()) {
                            str += "\tli t2, " + std::to_string(reg_table[src].first) + "\n";
                            str += "\tadd t1, t1, t2\n";
                            str += "\tadd t1, t1, sp\n";
                        }
                        else {
                            auto it = globl_vars.begin();
                            for (; it != globl_vars.end() && get_name(src->name) != *it; ++it);
                            if (it != globl_vars.end()) {
                                str += "\tla t2, " + get_name(src->name) + "\n";
                                str += "\tadd t1, t1, t2\n";
                            }
                            else {
                                str += "\tli t2, " + std::to_string(offset) + "\n";
                                str += "\tadd t1, t1, t2\n";
                                str += "\tadd t1, t1, sp\n";
                                reg_table[src] = std::pair<int, int>(offset, 0);
                                offset += len;
                            }
                        }
                        str += "\tli t4, " + std::to_string(offset) + "\n";
                        str += "\tadd t4, t4, sp\n";
                        // t1 is the absolute address of the element
                        str += "\tsw t1, 0(t4)\n";
                        reg_table[value] = std::pair<int, int>(offset, 0);
                        ptrs[value] = value_is_ptr(src) + 1;
                        std::cout << value_is_ptr(src) + 1 << std::endl;
                        offset += 4;
                        break;
                    }
                    default: break;
                }
            }
        }
        str += "\n";
    }
}