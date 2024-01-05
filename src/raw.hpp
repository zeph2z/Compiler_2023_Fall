#pragma once

#include <memory>
#include <string>
#include <cassert>
#include "koopa.h"

void raw2riscv(koopa_raw_program_t &raw, std::string &str) {
    str += "    .text\n";
    str += "    .globl main\n";

    for (size_t i = 0; i < raw.funcs.len; ++i) {
        assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
        koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];
        int _i = 1;
        while (func->name[_i] != 0) {
            str += func->name[_i];
            ++_i;
        }
        str += ":\n";
    
        for (size_t j = 0; j < func->bbs.len; ++j) {
            assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
            koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[j];
        
            for (size_t k = 0; k < bb->insts.len; ++k) {
                koopa_raw_value_t value = (koopa_raw_value_t) bb->insts.buffer[k];

                switch (value->kind.tag) {
                    case KOOPA_RVT_RETURN: {
                        koopa_raw_value_t ret_value = value->kind.data.ret.value;
                        int32_t int_val = ret_value->kind.data.integer.value;
                        str += "    li a0, " + std::to_string(int_val) + "\n";
                        str += "    ret\n";
                        break;
                    }
                }
            }
        }
    }
}