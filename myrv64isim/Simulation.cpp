#include "Simulation.h"
using namespace std;

extern void read_elf(char* filename);
extern unsigned int cadr;
extern unsigned int dadr;
extern unsigned int csize;
extern unsigned long long dsize;
extern unsigned int vadr;
extern unsigned long long gp;
extern unsigned int madr;
extern unsigned int endPC;
extern unsigned int entry;
extern unsigned long long global_pointer;
extern unsigned int sdata_offset;
extern unsigned long long sdata_adr,sdata_size;
extern FILE *file;

unsigned short read_mem_2(unsigned long long address);
unsigned int read_mem_4(unsigned long long address);
unsigned long long read_mem_8(unsigned long long address);
void write_mem_2(unsigned long long address, unsigned short val);
void write_mem_4(unsigned long long address, unsigned int val);
void write_mem_8(unsigned long long address, unsigned long long val);

unsigned short read_mem_2(unsigned long long address){
    return memory[address] | (memory[address+1]<<8);
}
unsigned int read_mem_4(unsigned long long address){
    return memory[address] | (memory[address+1]<<8) | (memory[address+2]<<16) | (memory[address+3]<<24);
}
unsigned long long read_mem_8(unsigned long long address){
    return memory[address] | (memory[address+1]<<8) | (memory[address+2]<<16) | (memory[address+3]<<24)
           | (memory[address+4]<<32) | (memory[address+5]<<40) | (memory[address+6]<<48) | (memory[address+7]<<56);
}

void write_mem_2(unsigned long long address, unsigned short val){
    memory[address] = (unsigned char)val;
    memory[address+1] = (unsigned char)(val>>8);
}
void write_mem_4(unsigned long long address, unsigned int val){
    memory[address] = (unsigned char)val;
    memory[address+1] = (unsigned char)(val>>8);
    memory[address+2] = (unsigned char)(val>>16);
    memory[address+3] = (unsigned char)(val>>24);
}
void write_mem_8(unsigned long long address, unsigned long long val){
    memory[address] = (unsigned char)val;
    memory[address+1] = (unsigned char)(val>>8);
    memory[address+2] = (unsigned char)(val>>16);
    memory[address+3] = (unsigned char)(val>>24);
    memory[address+4] = (unsigned char)(val>>32);
    memory[address+5] = (unsigned char)(val>>40);
    memory[address+6] = (unsigned char)(val>>48);
    memory[address+7] = (unsigned char)(val>>56);
}

void display_regs(){
    for(int i = 0; i < 32; i++){
        cout<<"x"<<i<<":\t"<<reg[i];
        if(i%2 == 1){
            cout<<endl;
        }
        else{
            cout<<"\t\t\t";
        }
    }
}

void disp_memory(int addr, int size, int blocks){
    if(size == 1){
        for(int i=0;i<blocks;i++){
            cout<<"addr:"<<addr<<"-----"<<(unsigned long long int)memory[addr]<<"\n";
            if((i+1)%16==0){
                cout<<endl;
            }
            addr += 1;
        }
        if(blocks%16!=0){
            cout<<endl;
        }
    }
    else if(size == 2){
        for(int i=0;i<blocks;i++){
            cout<<"addr:"<<addr<<"-----"<<(unsigned long long int)read_mem_2(addr)<<"\n";
            if((i+1)%8==0){
                cout<<endl;
            }
            addr += 2;
        }
        if(blocks%8!=0){
            cout<<endl;
        }
    }
    else if(size == 4){
        for(int i=0;i<blocks;i++){
            cout<<"addr:"<<addr<<"-----"<<(unsigned long long int)read_mem_4(addr)<<"\n";
            if((i+1)%8==0){
                cout<<endl;
            }
            addr += 4;
        }
        if(blocks%8!=0){
            cout<<endl;
        }
    }
    else if(size == 8){
        for(int i=0;i<blocks;i++){
            cout<<"addr:"<<addr<<"-----"<<(unsigned long long int)read_mem_8(addr)<<"\n";
            if((i+1)%4==0){
                cout<<endl;
            }
            addr += 8;
        }
        if(blocks%4!=0){
            cout<<endl;
        }
    }
    else{
        cout<<"Only 1,2,4,8 Byte are supported!"<<endl;
    }
}

void display_memory(){
    cout<<"Start Display Memory"<<endl;
    int addr, size, blocks;
    while(1){
        cout<<"enter addr, size(Byte), blocks"<<endl;
        cout<<"enter -1, -1, -1 to exit"<<endl;
        cin>>addr>>size>>blocks;
        if(addr==-1||size==-1||blocks==-1){
            break;
        }
        disp_memory(addr,size,blocks);
    }
    cout<<"Display Memory Over"<<endl;
}

void debug(){
    cout<<"debug:"<<endl;
    char ch;
    while(1){
        cin>>ch;
        if(ch == 'r'){
            display_regs();
        }
        if(ch == 'm'){
            display_regs();
        }
        if(ch == 'i'){
            cout<<(int) inst<<endl;
        }
        if(ch == 'e'){
            break;
        }
    }
}


bool print_inst = 0;
bool onestep = 0;
bool translate_detail = 0;
//指令运行数
long long inst_num=0;

//系统调用退出指示
int exit_flag=0;

int main(int argc, char* argv[])
{

    char filename[100] = {0};
    
    cout << "filename:" << endl;
    cin >> filename;

    if(filename == NULL)
    {
        cout<<"Error: No input file!"<<endl;
        return 0;
    }
    else
    {
        cout<<"Input file:"<<filename<<endl;
    }


    cout<<"print_inst?(1/0)"<<endl;
    cin>>print_inst;
    cout<<"onestep?(1/0)"<<endl;
    cin>>onestep;
    cout<<"translate detail?(1/0)"<<endl;
    cin>>translate_detail;
	//解析elf文件
	read_elf(filename);
	
	//加载内存
	load_memory();

	//设置入口地址
	PC=entry;
	
	//设置全局数据段地址寄存器
	reg[3]=global_pointer;
	
	reg[2]=MAX/2;//栈基址 （sp寄存器）
	
	//结束PC的设置
	int end=(int)endPC;

	while(PC!=end)
	{
        reg[0]=0;

        if(onestep){
            cout<<"PC:"<<PC<<endl;
        }

	    translate_inst();
	    
        execute_inst();

        if(onestep){
            cout<<"after execute_inst"<<endl;
            while(1){
                cout<<"enter r to display the regs, enter m to display memory,enter c to continue, enter e to close onestep"<<endl;
                char ch;
                cin>>ch;
                if(ch == 'r'){
                    display_regs();
                }
                if (ch == 'm'){
                    display_memory();
                }
                if(ch =='c'){
                    break;
                }
                if(ch =='e'){
                    onestep = 0;
                    break;
                }
            }
        }

        if(exit_flag==1)
            break;

        reg[0]=0;//一直为零

	}
    cout<<"Over! enter e to exit. enter c to check regs and memory"<<endl;
    char ch;
    cin>>ch;
    if(ch =='c'){
        while(1){
            cout<<"enter r to check regs, enter m to check memory, enter e to exit"<<endl;
            char ch2;
            cin>>ch2;
            if(ch2=='r'){
                display_regs();
            }
            if(ch2 == 'e'){
                break;
            }
            if(ch2 == 'm'){
                display_memory();
            }
        }
    }
    printf("Instruct Num: %ld\n",inst_num);

	return 0;
}

void translate_inst()
{
	inst=read_mem_4(PC);
	
	//rd=getbit(7,11);
    OP=getbit(inst,0,6);
    rd=getbit(inst,7,11);
    fuc3=getbit(inst,12,14);
    rs1=getbit(inst,15,19);
    rs2=getbit(inst,20,24);
    fuc7=getbit(inst,25,31);
	
    imm_I=getbit(inst,20,31);
    imm_S=getbit(inst,7,11)|(getbit(inst,25,31)<<5);
    imm_SB=getbit(inst,8,11)|(getbit(inst,25,30)<<4)|(getbit(inst,7,7)<<10)|(getbit(inst,31,31)<<11);
    imm_U=getbit(inst,12,31);
    imm_UJ=getbit(inst,21,30)|(getbit(inst,20,20)<<10)|(getbit(inst,12,19)<<11)|(getbit(inst,31,31)<<19);
    shamt = getbit(inst,20,25);

    if(translate_detail){
        cout<<"inst  :"<<(int) inst<<endl;
        cout<<"OP    :"<<OP<<endl;
        cout<<"rd    :"<<rd<<endl;
        cout<<"fuc3  :"<<fuc3<<endl;
        cout<<"rs1   :"<<rs1<<endl;
        cout<<"rs2   :"<<rs2<<endl;
        cout<<"fuc7  :"<<fuc7<<endl;
        cout<<"imm_I :"<<imm_I<<endl;
        cout<<"imm_S :"<<imm_S<<endl;
        cout<<"imm_SB:"<<imm_SB<<endl;
        cout<<"imm_U :"<<imm_U<<endl;
        cout<<"imm_UJ:"<<imm_UJ<<endl;
        cout<<"shamt :"<<shamt<<endl;
        cout<<"enter c to continue"<<endl;
        char ch;
        cin>>ch;
    }

	inst_num++;
	PC+=4;
}

//加载代码段
//初始化PC
void load_memory()
{
	fseek(file,cadr,SEEK_SET);
    fread(&memory[vadr],1,csize,file);
    fseek(file,dadr,SEEK_SET);
    fread(&memory[gp],1,dsize,file);
    fseek(file,sdata_offset,SEEK_SET);
    fread(&memory[sdata_adr], 1, sdata_size, file);
    fclose(file);
}

void execute_inst()
{
	if(OP==OP_R)
	{
		if(fuc3==F3_ADD&&fuc7==F7_ADD)
		{
            reg[rd]=reg[rs1]+reg[rs2];
            if(print_inst)
            {
                cout<<"add\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
		}
		else if(fuc3==F3_MUL&&fuc7==F7_MUL)
        {
            reg[rd]=reg[rs1]*reg[rs2];
            if(print_inst)
            {
                cout<<"mul\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
		else if(fuc3==F3_SUB&&fuc7==F7_SUB)
		{
            reg[rd]=reg[rs1]-reg[rs2];
            if(print_inst)
            {
                cout<<"sub\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SLL&&fuc7==F7_SLL)
        {
            reg[rd]=reg[rs1]<<reg[rs2];
            if(print_inst)
            {
                cout<<"sll\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_MULH&&fuc7==F7_MULH)
        {
            __uint128_t result = __uint128_t(reg[rs1])*__uint128_t(reg[rs2]);
            result = result >> 64;
            reg[rd]=result;
            if(print_inst)
            {
                cout<<"mulh\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SLT&&fuc7==F7_SLT)
        {
            reg[rd]=(reg[rs1]<reg[rs2])?1:0;
            if(print_inst)
            {
                cout<<"slt\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_XOR&&fuc7==F7_XOR)
        {
            reg[rd]=reg[rs1]^reg[rs2];
            if(print_inst)
            {
                cout<<"xor\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_DIV&&fuc7==F7_DIV)
        {
            reg[rd]=reg[rs1]/reg[rs2];
            if(print_inst)
            {
                cout<<"div\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SRL&&fuc7==F7_SRL)
        {
            reg[rd]=reg[rs1]>>reg[rs2];
            if(print_inst)
            {
                cout<<"srl\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SRA&&fuc7==F7_SRA)
        {
            long long int reg_rs1_signed = reg[rs1];
            reg[rd]= reg_rs1_signed >>reg[rs2];
            if(print_inst)
            {
                cout<<"sra\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_OR&&fuc7==F7_OR)
        {
            reg[rd]=reg[rs1]|reg[rs2];
            if(print_inst)
            {
                cout<<"or\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_REM&&fuc7==F7_REM)
        {
            reg[rd]=reg[rs1]%reg[rs2];
            if(print_inst)
            {
                cout<<"rem\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_AND&&fuc7==F7_AND)
        {
            reg[rd]=reg[rs1]&reg[rs2];
            if(print_inst)
            {
                cout<<"and\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
    }
    else if(OP==OP_RW){
        if(fuc3==F3_ADDW&&fuc7==F7_ADDW)
        {
            int reg_rd = (reg[rs1]&0xFFFFFFFF) + (reg[rs2]&0xFFFFFFFF);
            reg[rd] = ext_signed_64(reg_rd, 1, 32);
            if(print_inst)
            {
                cout<<"addw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SUBW&&fuc7==F7_SUBW)
        {
            int reg_rd =(reg[rs1]&0xFFFFFFFF) - (reg[rs2]&0xFFFFFFFF);
            reg[rd] = ext_signed_64(reg_rd, 1, 32);
            if(print_inst)
            {
                cout<<"subw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_MULW&&fuc7==F7_MULW)
        {
            int reg_rd = ((reg[rs1]&0xFFFFFFFF) * (reg[rs2]&0xFFFFFFFF))&0xFFFFFFFF;
            reg[rd] = ext_signed_64(reg_rd, 1, 32);
            if(print_inst)
            {
                cout<<"mulw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_DIVW&&fuc7==F7_DIVW)
        {
            int reg_rd =((reg[rs1]&0xFFFFFFFF) / (reg[rs2]&0xFFFFFFFF))&0xFFFFFFFF;
            reg[rd] = ext_signed_64(reg_rd, 1, 32);
            if(print_inst)
            {
                cout<<"divw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_REMW&&fuc7==F7_REMW)
        {
            int reg_rd =((reg[rs1]&0xFFFFFFFF) % (reg[rs2]&0xFFFFFFFF))&0xFFFFFFFF;
            reg[rd] = ext_signed_64(reg_rd, 1, 32);
            if(print_inst)
            {
                cout<<"remw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
    }
    else if(OP==OP_LW)
    {
        if(fuc3==F3_LB)
        {
            long long int address = reg[rs1] + ext_signed_64(imm_I,1,12);
            reg[rd] = ext_signed_64(memory[address], 1, 8);
            if(print_inst)
            {
                cout<<"lb\t"<<"x"<<rd<<","<<ext_signed_64(imm_I,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_LH)
        {
            long long int address = reg[rs1] + ext_signed_64(imm_I,1,12);
            reg[rd] = ext_signed_64(read_mem_2(address), 1, 16);
            if(print_inst)
            {
                cout<<"lh\t"<<"x"<<rd<<","<<ext_signed_64(imm_I,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_LW)
        {
            long long int address = reg[rs1] + ext_signed_64(imm_I,1,12);
            reg[rd] = ext_signed_64(read_mem_4(address), 1, 32);
            if(print_inst)
            {
                cout<<"lw\t"<<"x"<<rd<<","<<ext_signed_64(imm_I,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_LD)
        {
            long long int address = reg[rs1] + ext_signed_64(imm_I,1,12);
            reg[rd] = read_mem_8(address);
            if(print_inst)
            {
                cout<<"ld\t"<<"x"<<rd<<","<<ext_signed_64(imm_I,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
    }
	else if(OP==OP_I)
    {
        if(fuc3==F3_ADDI)
        {
            reg[rd] = reg[rs1] + ext_signed_64(imm_I,1,12);
            if(print_inst)
            {
                cout<<"addi\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,1,12)<<endl;
            }
        }
        else if(fuc3==F3_SLLI&&fuc7==F7_SLLI)
        {
           reg[rd] = reg[rs1] << shamt;
           if(print_inst)
            {
                cout<<"slli\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<shamt<<endl;
            }
        }
        else if(fuc3==F3_SLTI)
        {
            reg[rd] = (reg[rs1] < ext_signed_64(imm_I,1,12))?1:0;
            if(print_inst)
            {
                cout<<"slti\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,1,12)<<endl;
            }
        }
        else if(fuc3==F3_XORI)
        {
            reg[rd] = reg[rs1] ^ ext_signed_64(imm_I,0,12);
            if(print_inst)
            {
                cout<<"xori\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,0,12)<<endl;
            }
        }
        else if(fuc3==F3_SRLI&&fuc7==F7_SRLI)
        {
            reg[rd] = reg[rs1] >> shamt;
            if(print_inst)
            {
                cout<<"srli\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<shamt<<endl;
            }
        }
        else if(fuc3==F3_SRAI&&fuc7==F7_SRAI)
        {
            long long int reg_rs1_signed = reg[rs1];
            reg[rd] = reg_rs1_signed >> shamt;
            if(print_inst)
            {
                cout<<"srai\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<shamt<<endl;
            }
        }
        else if(fuc3==F3_ORI)
        {
            reg[rd] = reg[rs1] | ext_signed_64(imm_I,0,12);
            if(print_inst)
            {
                cout<<"ori\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,0,12)<<endl;
            }
        }
        else if(fuc3==F3_ANDI)
        {
            reg[rd] = reg[rs1] & ext_signed_64(imm_I,0,12);
            if(print_inst)
            {
                cout<<"andi\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,0,12)<<endl;
            }
        }
    }
    else if(OP==OP_IW)
    {
        if(fuc3==F3_ADDIW)
        {
            int reg_rd = (reg[rs1]&0xFFFFFFFF) + ext_signed(imm_I,1,12);
            reg[rd] = ext_signed_64(reg_rd, 1, 32);
            if(print_inst)
            {
                cout<<"addiw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed(imm_I,1,12)<<endl;
            }
        }
    }
    else if(OP==OP_JALR)
    {
        if(fuc3==F3_JALR)
        {
            reg[rd] = PC;
            PC = (reg[rs1] + ext_signed_64(imm_I,0,12)) &(-2);
            if(print_inst)
            {
                cout<<"jalr\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,1,12)<<endl;
            }
        }
    }
    //!!!!!!!!!!!!!!!!!!!!!!!!//
    else if(OP==OP_SCALL)//系统调用指令
    {
        if(fuc3==F3_SCALL&&fuc7==F7_SCALL)
        {
            if(print_inst)
            {
                cout<<"SCALL:";
            }
            if(reg[17] == 0)
            {
                printf("%s", &memory[reg[10]]);
                if(print_inst)
                {
                    cout<<"print_str"<<endl;
                }
            }
            else if(reg[17] == 1)
            {
                printf("%c", (char)reg[10]);
                if(print_inst)
                {
                    cout<<"print_char"<<endl;
                }
            }
            else if(reg[17] == 2)
            {
                printf("%d", (int) reg[10]);
                if(print_inst)
                {
                    cout<<"print_int"<<endl;
                }
            }
            else if(reg[17] == 3)
            {
                scanf("%d", &memory[reg[10]]);
                if(print_inst)
                {
                    cout<<"scan_int"<<endl;
                }
            }
            else if(reg[17] == 4)
            {
                exit_flag = 1;
                if(print_inst)
                {
                    cout<<"my_exit"<<endl;
                }
            }
            else if(reg[17] == 5)
            {
                time_t seconds;
                seconds = time(NULL);
                memory[reg[10]] = (long long int)seconds;
                if(print_inst)
                {
                    cout<<"time"<<endl;
                }
            }
            else
            {
                cout<<"OTHER SCALL: REG17 ="<<reg[17]<<endl;
            }
        }
    }
    //!!!!!!!!!!!!!!!!!!!!//

    else if(OP==OP_SW)
    {
        if(fuc3==F3_SB)
        {
            memory[reg[rs1]+ext_signed_64(imm_S,1,12)] = (unsigned char)reg[rs2];
            if(print_inst)
            {
                cout<<"sb\t"<<"x"<<rs2<<","<<ext_signed_64(imm_S,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_SH)
        {
            write_mem_2(reg[rs1]+ext_signed_64(imm_S,1,12), (unsigned short)reg[rs2]);
            if(print_inst)
            {
                cout<<"sh\t"<<"x"<<rs2<<","<<ext_signed_64(imm_S,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_SW)
        {
            write_mem_4(reg[rs1]+ext_signed_64(imm_S,1,12), (unsigned int)reg[rs2]);
            if(print_inst)
            {
                cout<<"sw\t"<<"x"<<rs2<<","<<ext_signed_64(imm_S,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_SD)
        {
            write_mem_8(reg[rs1]+ext_signed_64(imm_S,1,12), reg[rs2]);
            if(print_inst)
            {
                cout<<"sd\t"<<"x"<<rs2<<","<<ext_signed_64(imm_S,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
    }
    else if(OP==OP_BEQ)
    {
        if(fuc3==F3_BEQ)
        {
            if(reg[rs1] == reg[rs2])
            {
                PC = PC - 4 + (ext_signed(imm_SB,1,12)<<1);
            }
            if(print_inst)
            {
                cout<<"beq\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_BNE)
        {
            if(reg[rs1] != reg[rs2])
            {
                PC = PC - 4 + (ext_signed(imm_SB,1,12)<<1);
            }
            if(print_inst)
            {
                cout<<"bne\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_BLT)
        {
            if((long long int)(reg[rs1]) < (long long int)(reg[rs2]))
            {
                PC = PC - 4 + (ext_signed(imm_SB,1,12)<<1);
            }
            if(print_inst)
            {
                cout<<"blt\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_BGE)
        {
            if((long long int)(reg[rs1]) >= (long long int)(reg[rs2]))
            {
                PC = PC - 4 + (ext_signed(imm_SB,1,12)<<1);
            }
            if(print_inst)
            {
                cout<<"bge\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
    }
    else if(OP==OP_AUIPC)
    {
        if(1){
            reg[rd] = PC - 4 + (ext_signed_64(imm_U,1,20)<<12);
            if(print_inst)
            {
                cout<<"auipc\t"<<"x"<<rd<<","<<ext_signed_64(imm_U,1,20)<<endl;
            }
        }
    }
    else if(OP==OP_LUI)
    {
        if(1){
            reg[rd] = ext_signed_64(imm_U,1,20)<<12;
            if(print_inst)
            {
                cout<<"lui\t"<<"x"<<rd<<","<<ext_signed_64(imm_U,1,20)<<endl;
            }
        }
    }
    else if(OP==OP_JAL)
    {
        if(1){
            reg[rd] = PC;
            PC = PC - 4 + (ext_signed(imm_UJ,1,20) << 1);
            if(print_inst)
            {
                cout<<"jal\t"<<"x"<<rd<<","<<ext_signed(imm_UJ,1,20)<<endl;
            }
        }
    }
    else
    {
        cout<<"ERROR inst:"<<inst<<endl;
        debug();
    }
}