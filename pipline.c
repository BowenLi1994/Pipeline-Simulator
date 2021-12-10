
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


struct instructions {
    char* instruction;
    char* type;
    int rs;
    int rt;
    int rd;
    int imm;
    int branch_target;
};

struct if_id {
    struct instructions ins;
    int pc_4;

};

struct id_ex {
    struct instructions ins;
    int pc_4;
    int branch_target;
    int rs;
    int rt;
    int rd;
    int readData1;
    int readData2;
    int imm;
};

struct ex_mem {

    struct instructions ins;
    int alu_result;
    int write_data;
    int write_register;

};

struct mem_wb {
    struct instructions ins;
    int write_data_mem;
    int write_data_alu;
    int write_register;
};

struct state {
    struct if_id r1;
    struct id_ex r2;
    struct ex_mem r3;
    struct mem_wb r4;

};

struct branch_predictor {
    int pc;
    int branch_target;
    int state;
};



void tanslate_function(int* machine_code, struct instructions* ins_set, int instruction_number, struct branch_predictor* prediction_set);
int bitExtracted(int number, int begin, int end);
int extract_opcode(int machine_code);
int extract_rs(int machine_code);
int extract_rt(int machine_code);
int extract_rd(int machine_code);
int extract_imm(int machine_code);
int extract_shamt(int machine_code);
int extract_funct(int machine_code);
char instruction_type(int op_code);
char* generate_i_instruction(int op_code, int rs, int rt, int imm);
char* generate_r_instruction(int op_code, int rs, int rt, int rd, int shamt, int funct);
char* get_register_name(int reg_num);
void print_state(struct state s, int pc, int* data_memory, int* register_set);
void init_state(struct state* s);
void init_r1(struct if_id* r1);
void init_r2(struct id_ex* r2);
void init_r3(struct ex_mem* r3);
void init_r4(struct mem_wb* r4);
int alu_calculation(struct id_ex r2, int* data_mem, int* reg_set);
void init_ins(struct instructions* ins);

int main(int argc, const char* argv[]) {


    //instructions set;
    struct instructions instructions_set[100];

    // predictor set;
    struct branch_predictor brach_set[100];


    //machine code read from file
    int machine_insructions[100];
    int instructions_number = 0;
    //line
    char line[200][256];
    int line_number = 0;


    //register set
    int register_set[32] = { 0 };
    //data memory set
    int data_memory[32] = { 0 };
    int data_number = 0;

    //calculate the start address of memory data
    int data_flag = 0;
    int datamemory_add = 0;
    int temp_address = -4;

    //reading file
//    FILE* fp;
//    fp=fopen("./result4.asm", "r");



  
    while (fgets(line[line_number], 256, stdin) != NULL) {

            temp_address += 4;
            //printf("temp_address: %d line number: %d\n", temp_address,line_number);


            if (line[line_number][0] == '\n') {

                data_flag = 1;
                datamemory_add = temp_address;
            }


            //printf("Line %d: %s", line_number, line[line_number]);

            if (data_flag == 0) {
                machine_insructions[instructions_number] = atoi(line[line_number]);
                instructions_number++;
            }
            if (data_flag == 1 && line[line_number][0] != '\n') {
                data_memory[data_number] = atoi(line[line_number]);
                data_number++;
            }


            line_number++;

        }
 

    tanslate_function(machine_insructions, instructions_set, instructions_number, brach_set);
    //printf("hello!\n");

     int pc = 0;
     struct state old_state;
     init_state(&old_state);
     struct state new_state;
     init_state(&new_state);
     int j;
     int number_branches=0;
    for(j=0;j<instructions_number;j++){
        if(strcmp(instructions_set[j].type,"bne")==0)
            number_branches++;
    }

     int cycle = 1;
     int counter = 0;
     int halt_flag = 0;
     int forwarda_flag = 0;
     int forwardb_flag = 0;
     int forwarda2_flag = 0;
     int forwardb2_flag = 0;
     int temp_alu_result=0;
     int stall_flag = 0;
     int write_cycle = 0;
     int write_number=0;
    
    int number_stalls=0;
    int mis_branch=0;
     
     while (1) {

         printf("********************\n");
         printf("state at the beginning of cycle %d\n", cycle);
         print_state(new_state, pc, data_memory, register_set);

         //if (ID / EX.MemRead &&
         //    ((ID / EX.RegisterRt == IF / ID.RegisterRs) ||
         //        (ID / EX.RegisterRt == IF / ID.RegisterRt)))
         if ((strcmp(old_state.r2.ins.type, "lw") == 0 || strcmp(old_state.r2.ins.type, "sw") == 0) && (old_state.r2.rt == old_state.r1.ins.rs || old_state.r2.rt == old_state.r1.ins.rt)) {
             stall_flag = 1;
             number_stalls++;
             /* printf("stall!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
              printf("old_state.r2.rt: %d\n", old_state.r2.rt);*/
         }


         if (strcmp(new_state.r4.ins.instruction, "halt") == 0)
             break;

         //control hazard
         if (strcmp(new_state.r1.ins.instruction, "bne") == 0) {
             if (brach_set[counter--].state >= 1) {
             init_r1(&(old_state.r1));
             }
         }

         //r4->r3
         old_state.r4.ins = old_state.r3.ins;
         old_state.r4.write_data_mem = old_state.r3.write_data;
         old_state.r4.write_data_alu = old_state.r3.alu_result;
         old_state.r4.write_register = old_state.r3.write_register;
         if (old_state.r4.write_data_alu != 0 && old_state.r4.write_register != 0) {
             write_cycle = cycle;
             //printf("************writing to reg: %d\n", write_cycle);
             register_set[old_state.r4.write_register] = old_state.r4.write_data_alu;
         }
         if (cycle == write_cycle + 1) {
             register_set[old_state.r4.write_register] = write_number;
         }
         if (strcmp(old_state.r4.ins.type, "lw") == 0){
             if (old_state.r4.write_register != 0) {
                 register_set[old_state.r4.write_register] = data_memory[old_state.r4.ins.imm/4];
             }
             old_state.r4.write_data_mem = data_memory[old_state.r4.ins.imm/4];
         }

            

        //r3->r2
         old_state.r3.ins = old_state.r2.ins;
         old_state.r3.alu_result = alu_calculation(old_state.r2,data_memory,register_set);
         if (old_state.r2.rd != 0)
             old_state.r3.write_register = old_state.r2.rd;
         else
             old_state.r3.write_register = old_state.r2.rt;
        /* if (EX / MEM.RegWrite && EX / MEM.WriteReg != 0 &&
             EX / MEM.WriteReg == ID / EX.RegisterRs)*/
         if (forwarda_flag == 1) {
             old_state.r3.alu_result = temp_alu_result+old_state.r3.ins.imm;
             forwarda_flag = 0;
         }
         if (forwardb_flag == 1) {
             old_state.r3.alu_result = temp_alu_result+old_state.r3.ins.imm;
             forwardb_flag = 0;
         }
         if (forwarda2_flag == 1) {
             old_state.r3.alu_result = temp_alu_result + old_state.r3.ins.imm;
             forwarda2_flag = 0;
         }
         if (forwardb2_flag == 1) {
             old_state.r3.alu_result = temp_alu_result + old_state.r3.ins.imm;
             forwardb2_flag = 0;
         }






         //r2->r1
         old_state.r2.ins = old_state.r1.ins;
         old_state.r2.pc_4 = old_state.r1.pc_4;
         old_state.r2.branch_target = old_state.r1.ins.branch_target;
         old_state.r2.rs = old_state.r1.ins.rs;
         old_state.r2.rt = old_state.r1.ins.rt;
         old_state.r2.rd = old_state.r1.ins.rd;
         old_state.r2.imm = old_state.r1.ins.imm;
         old_state.r2.readData1 = register_set[old_state.r1.ins.rs];
         old_state.r2.readData2 = register_set[old_state.r1.ins.rt];


  



         //r1->file
        if (counter >= instructions_number) {
             struct instructions noop;
             init_ins(&noop);
             old_state.r1.ins = noop;
             old_state.r1.pc_4 = pc + 4;
         }
         if (halt_flag == 0) {
             //printf("halt no\n");
             old_state.r1.ins = instructions_set[counter];
             old_state.r1.pc_4 = pc + 4;
         }
         else {
             struct instructions noop;
             init_ins(&noop);
             old_state.r1.ins = noop;
             old_state.r1.pc_4 = pc + 4;
         }

         //data hazard
         if (old_state.r3.write_register != 0 && (old_state.r3.ins.rt != 0|| old_state.r3.ins.rs != 0|| old_state.r3.ins.rd != 0) && (old_state.r3.write_register == old_state.r2.rs)) {
            // printf("\n\n\ntrig forwarding: *********************\n\n\n");
             temp_alu_result = old_state.r3.alu_result;
             forwarda_flag = 1;
         }

         if (old_state.r3.write_register != 0 && (old_state.r3.ins.rt != 0|| old_state.r3.ins.rs != 0|| old_state.r3.ins.rd != 0) && (old_state.r3.write_register == old_state.r2.rt)) {
             //printf("\n\n\ntrig forwarding: *********************\n\n\n");
             temp_alu_result = old_state.r3.alu_result;
             forwardb_flag = 1;
         }

         //• 2a. if (MEM / WB.RegWrite && MEM / WB.WriteReg != 0 &&
         //    MEM / WB.WriteReg == ID / EX.RegisterRs) ForwardA = 01
         if (old_state.r4.write_register != 0 && (old_state.r4.ins.rt != 0 || old_state.r4.ins.rs != 0 || old_state.r4.ins.rd != 0) && (old_state.r4.write_register == old_state.r2.rt)) {
             temp_alu_result = old_state.r4.write_data_alu;
             forwarda2_flag = 1;
         }
         if (old_state.r4.write_register != 0 && (old_state.r4.ins.rt != 0 || old_state.r4.ins.rs != 0 || old_state.r4.ins.rd != 0) && (old_state.r4.write_register == old_state.r2.rs)) {
             temp_alu_result = old_state.r4.write_data_alu;
             forwarda2_flag = 1;
         }

         //stall
         if (stall_flag == 1) {
             counter--;
             pc -= 4;
             old_state.r1 = new_state.r1;

             init_r2(&(old_state.r2));
             old_state.r2.pc_4 = old_state.r1.pc_4;
             old_state.r2.branch_target = old_state.r1.pc_4;
             old_state.r2.readData1 = 0;
             old_state.r2.rs = 0;
             old_state.r2.rt = 0;
             old_state.r2.rd = 0;
            
             
             stall_flag = 0;
         }

         //control
         if (strcmp(old_state.r3.ins.instruction, "bne")==0) {
             //correct predict
             if (brach_set[(old_state.r1.pc_4 - 4) / 4].state <= 1 && register_set[old_state.r3.ins.rs] != register_set[old_state.r3.ins.rt]) {
                 brach_set[(old_state.r1.pc_4 - 4) / 4].state = 0;
             }
             if (brach_set[(old_state.r1.pc_4 - 4) / 4].state >1 && register_set[old_state.r3.ins.rs] == register_set[old_state.r3.ins.rt]) {
                 brach_set[(old_state.r1.pc_4 - 4) / 4].state = 3;
             }
             //wrong predict
             if (brach_set[(old_state.r1.pc_4 - 4) / 4].state <= 1 && register_set[old_state.r3.ins.rs] == register_set[old_state.r3.ins.rt]) {
                 brach_set[(old_state.r1.pc_4 - 4) / 4].state++;
                 init_r1(&(old_state.r1));
                 init_r2(&(old_state.r2));
                 mis_branch++;
             }

             if (brach_set[(old_state.r1.pc_4 - 4) / 4].state > 1 && register_set[old_state.r3.ins.rs] != register_set[old_state.r3.ins.rt]) {
                 brach_set[(old_state.r1.pc_4 - 4) / 4].state--;
                 init_r1(&(old_state.r1));
                 init_r2(&(old_state.r2));
                 mis_branch++;
             }
         }




         //if (strcmp(old_state.r3.ins.instruction, "ori $s0,$0,24") == 0) {
         //    printf("\n\n\ntrig forwarding: *********************\n\n\n");
         //    printf("old_state.r3.write_register: %d\n", old_state.r3.write_register);
         //    printf("old_state.r3.ins.rs: %d\n", old_state.r3.ins.rs);
         //    printf("old_state.r3.ins.rt: %d\n", old_state.r3.ins.rt);
         //    printf("old_state.r2.rs: %d\n", old_state.r2.rs);
         //}






         //printf("r1 insruction: %s", old_state.r1.ins.instruction);
         if (strcmp(old_state.r1.ins.instruction, "halt") == 0)
             halt_flag = 1;

         pc += 4;

         counter++;

         new_state = old_state;
         cycle++;

     }
    printf("********************\n");
    printf("Total number of cycles executed: %d\n",cycle);
    printf("Total number of stalls: %d\n",number_stalls);
    printf("Total number of branches: %d\n",number_branches);
    printf("Total number of mispredicted branches: %d\n",mis_branch);







    return 0;

}


void tanslate_function(int* machine_code, struct instructions* ins_set, int instruction_number, struct branch_predictor* bp_set) {
    int i;
    for ( i=0 ; i < instruction_number; i++) {

        if (machine_code[i] != 0 && machine_code[i] != 1) {
            int op_code = extract_opcode(machine_code[i]);
            char data_type = instruction_type(op_code);
            char* instruction;

            if (data_type == 'i') {
                int rt = extract_rt(machine_code[i]);
                int rs = extract_rs(machine_code[i]);
                int imm = extract_imm(machine_code[i]);
                instruction = generate_i_instruction(op_code, rs, rt, imm);
                //struct instructions ins;
                ins_set[i].instruction = instruction;
                if (op_code == 5)
                    ins_set[i].type = "bne";
                if (op_code == 13)
                    ins_set[i].type = "ori";
                if (op_code == 12)
                    ins_set[i].type = "andi";
                if (op_code == 43)
                    ins_set[i].type = "sw";
                if (op_code == 35)
                    ins_set[i].type = "lw";


                ins_set[i].rs = rs;
                ins_set[i].rt = rt;
                ins_set[i].rd = 0;
                ins_set[i].imm = imm;
                int target_branch = 0;
                if (op_code == 35 || op_code == 43) {
                    target_branch = imm * 4 + i*4+4;
                }
                else if (op_code == 12 || op_code == 13)
                    target_branch = i * 4 + imm * 4 + 4;

                ins_set[i].branch_target = target_branch;
                if (op_code == 5) {
                    bp_set[i].pc = i * 4;
                    bp_set[i].branch_target = target_branch;
                    bp_set[i].state = 1;
                }



            }
            if (data_type == 'r') {
                int rd = extract_rd(machine_code[i]);
                int rs = extract_rs(machine_code[i]);
                int rt = extract_rt(machine_code[i]);
                int shamt = extract_shamt(machine_code[i]);
                int funct = extract_funct(machine_code[i]);
                int target_branch = 4 * i + 4;
                instruction = generate_r_instruction(op_code, rs, rt, rd, shamt, funct);
                ins_set[i].instruction = instruction;
                if (funct == 32)
                    ins_set[i].type = "add";
                if (funct == 34)
                    ins_set[i].type = "sll";
                if (funct == 0)
                    ins_set[i].type = "add";

                ins_set[i].rs = rs;
                ins_set[i].rt = rt;
                ins_set[i].rd = rd;
                ins_set[i].imm = 0;
                ins_set[i].branch_target = target_branch;
            }




        }

        if (machine_code[i] == 1) {
            init_ins(&ins_set[i]);
            ins_set[i].instruction = "halt";

        }
        if (machine_code[i] == 0) {
            init_ins(&ins_set[i]);
        }


    }

}


//https://www.geeksforgeeks.org/extract-k-bits-given-position-number/
int bitExtracted(int number, int k, int p)
{
    return (((1 << k) - 1) & (number >> (p - 1)));
}

int extract_opcode(int machine_code) {

    int op_code = bitExtracted(machine_code, 6, 27);
    if (op_code < 0)
        op_code += 64;

    return op_code;
}

int extract_rs(int machine_code) {

    int rs = bitExtracted(machine_code, 5, 22);
    if (rs < 0)
        rs += 32;

    return rs;

}

int extract_rt(int machine_code) {

    int rt = bitExtracted(machine_code, 5, 17);
    if (rt < 0)
        rt += 32;

    return rt;

}

int extract_imm(int machine_code) {

    int imm = bitExtracted(machine_code, 16, 1);
    if (imm < 0)
        imm = imm & 65535;

    return imm;

}

int extract_rd(int machine_code) {

    int rd = bitExtracted(machine_code, 5, 12);
    if (rd < 0)
        rd = rd & 65535;

    return rd;

}

int extract_shamt(int machine_code) {

    int shamt = bitExtracted(machine_code, 5, 7);
    if (shamt < 0)
        shamt = shamt & 65535;

    return shamt;

}

int extract_funct(int machine_code) {

    int funct = bitExtracted(machine_code, 6, 1);


    return funct;

}


char instruction_type(int op_code) {
    if (op_code == 0)
        return 'r';
    else
        return 'i';

}

char* generate_r_instruction(int op_code, int rs, int rt, int rd, int shamt, int funct) {

    char* instruction_name = (char*)malloc(5);

    switch (funct)
    {
    case 32:
        strcpy(instruction_name, "add");
        break;
    case 34:
        strcpy(instruction_name, "sub");
        break;
    case 0:
        strcpy(instruction_name, "sll");
        break;
    default:
        break;
    }

    char* instruction = (char*)malloc(50);
    char* rs_name;
    char* rt_name;
    char* rd_name;

    if (funct == 32 || funct == 34) {
        rs_name = get_register_name(rs);
        rt_name = get_register_name(rt);
        rd_name = get_register_name(rd);

        sprintf(instruction, "%s %s,%s,%s", instruction_name, rd_name, rs_name, rt_name);

    }

    if (funct == 0) {

        rt_name = get_register_name(rt);
        rd_name = get_register_name(rd);
        sprintf(instruction, "%s %s,%s,%d", instruction_name, rd_name, rt_name, shamt);
    }

    return instruction;

    //return 0;
}

char* get_register_name(int reg_num) {

    char* register_name = (char*)malloc(5);
    //printf("reg num: %d\n", reg_num);


    if (reg_num == 0)
        strcpy(register_name, "$0");
    else if (reg_num >= 8 && reg_num <= 15) {

        sprintf(register_name, "$t%d", reg_num - 8);

    }
    else if (reg_num >= 16 && reg_num <= 23) {
        sprintf(register_name, "$s%d", reg_num - 16);
    }



    return register_name;
}

char* generate_i_instruction(int op_code, int rs, int rt, int imm) {

    char* instruction = (char*)malloc(50);
    char* rs_name;
    char* rt_name;
    rs_name = get_register_name(rs);
    rt_name = get_register_name(rt);

    if (op_code == 12) {
        sprintf(instruction, "andi %s,%s,%d", rt_name, rs_name, imm);
    }
    if (op_code == 13) {
        sprintf(instruction, "ori %s,%s,%d", rt_name, rs_name, imm);
    }
    if (op_code == 5) {
        sprintf(instruction, "bne %s,%s,%d", rs_name, rt_name, imm);
    }
    if (op_code == 35) {
        sprintf(instruction, "lw %s,%d(%s)", rt_name, imm, rs_name);
    }
    if (op_code == 43) {
        sprintf(instruction, "sw %s,%d(%s)", rt_name, imm, rs_name);
    }


    return instruction;
}

void print_state(struct state s, int pc, int* data_memory, int* register_set) {
    printf("PC = %d\n", pc);
    printf("Data Memory :\n");
    int i;
    for (i = 0; i < 16; i++) {
        printf("\tdataMem[%d] = %d \t dataMem[%d] = %d\n", i, data_memory[i], i + 16, data_memory[i + 16]);
    }
    printf("Registers :\n");
    //int i;
    for (i = 0; i < 16; i++) {
        printf("\tregFile[%d] = %d \t regFile[%d] = %d\n", i, register_set[i], i + 16, register_set[i + 16]);
    }

    printf("IF/ID :\n");
    printf("\tInstruction : %s\n", s.r1.ins.instruction);
    printf("\tPCPlus4 : %d\n", s.r1.pc_4);
    printf("ID/EX :\n");
    printf("\tInstruction : %s\n", s.r2.ins.instruction);
    printf("\tPCPlus4 : %d\n", s.r2.pc_4);
    printf("\tbranchTarget : %d\n", s.r2.branch_target);
    printf("\treadData1 : %d\n", s.r2.readData1);
    printf("\treadData2 : %d\n", s.r2.readData2);
    printf("\timmed : %d\n", s.r2.imm);

    char* rs_name = get_register_name(s.r2.rs);
    rs_name++;
    printf("\trs : %s\n", rs_name);

    char* rt_name = get_register_name(s.r2.rt);
    rt_name++;
    printf("\trt : %s\n", rt_name);

    char* rd_name = get_register_name(s.r2.rd);
    rd_name++;
    printf("\trd : %s\n", rd_name);

    printf("EX/MEM :\n");
    printf("\tInstruction : %s\n", s.r3.ins.instruction);
    printf("\taluResult : %d\n", s.r3.alu_result);
    printf("\twriteDataReg : %d\n", s.r3.write_data);

    char* write_register_name = get_register_name(s.r3.write_register);
    write_register_name++;
    printf("\twriteReg : %s\n", write_register_name);


    printf("MEM/WB :\n");
    printf("\tInstruction : %s\n", s.r4.ins.instruction);
    printf("\twriteDataMem : %d\n", s.r4.write_data_mem);
    printf("\twriteDataALU : %d\n", s.r4.write_data_alu);
    char* writeReg_name = get_register_name(s.r4.write_register);
    writeReg_name++;
    printf("\twriteReg :  %s\n", writeReg_name);

    


}

void init_state(struct state* s) {

    init_r1(&(s->r1));
    init_r2(&(s->r2));
    init_r3(&(s->r3));
    init_r4(&(s->r4));

}

void init_r1(struct if_id* r1) {
    init_ins(&(r1->ins));
    r1->pc_4 = 0;
}

void init_r2(struct id_ex* r2) {
    init_ins(&(r2->ins));
    r2->branch_target = 0;
    r2->imm = 0;
    r2->pc_4 = 0;
    r2->rd = 0;
    r2->readData1 = 0;
    r2->readData2 = 0;
    r2->rs = 0;
    r2->rt = 0;
}

void init_r3(struct ex_mem* r3) {
    init_ins(&(r3->ins));
    r3->alu_result = 0;
    r3->write_data = 0;
    r3->write_register = 0;
}

void init_r4(struct mem_wb* r4) {
    init_ins(&(r4->ins));
    r4->write_data_alu = 0;
    r4->write_data_mem = 0;
    r4->write_register = 0;

}

int alu_calculation(struct id_ex r2, int* data_mem, int* reg_set) {

    int alu_result = 0;
    //printf("ALU Calulating: %s   %s\n", ins.instruction,);

    if (strcmp(r2.ins.instruction, "add") == 0) {
        alu_result = reg_set[r2.ins.rs] + reg_set[r2.ins.rt];
    }

    if (strcmp(r2.ins.type, "ori") == 0) {
        alu_result = r2.ins.imm | reg_set[r2.ins.rs];
    }

    if (strcmp(r2.ins.type, "andi") == 0) {
        alu_result = r2.ins.imm & reg_set[r2.ins.rs];
    }

   if (strcmp(r2.ins.instruction, "sub") == 0) {
        alu_result = reg_set[r2.rs] - reg_set[r2.ins.rt];
    }

    if (strcmp(r2.ins.instruction, "lw") == 0) {
        alu_result = reg_set[r2.ins.rs+(r2.ins.imm/4)];
    }

    if (strcmp(r2.ins.instruction, "sw") == 0) {
        alu_result = alu_result +reg_set[r2.ins.rs + (r2.ins.imm / 4)];
    }

    if (strcmp(r2.ins.instruction, "sll") == 0) {
        alu_result = r2.ins.rt<r2.ins.imm;
    }

    if (strcmp(r2.ins.instruction, "bne") == 0) {
        alu_result =0;
    }








    return alu_result;
}

void init_ins(struct instructions* ins) {
    ins->instruction = "NOOP";
    ins->branch_target = 0;
    ins->imm = 0;
    ins->rd = 0;
    ins->rs = 0;
    ins->rt = 0;
    ins->type = "";

}
