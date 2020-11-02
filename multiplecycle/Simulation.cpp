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
            cout<<(int)IFID.inst<<endl;
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
//
long long int cycles = 0;
struct PreInst{
    INST_TYPE inst_type;
    ALUOP aluop;
    int rs1, rs2, rd;
}preinst;
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
    //cout<<"translate detail?(1/0)"<<endl;
    //cin>>translate_detail;
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

	    //translate_inst();
        //execute_inst();
        IF();
        ID();
        if(IDEX.step == IIEW){
        	EX();
        	MEMWB.rd = EXMEM.rd;
        	MEMWB.MEMout = EXMEM.EXout;
        	WB();
        }
        else if(IDEX.step == IIEMW){
        	EX();
        	MEM();
        	WB();
        }
        else if(IDEX.step == IIEM){
        	EX();
        	MEM();
        }
        else if(IDEX.step == IIE){
        	EX();
        }


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
    cout<<"Cycles:"<<cycles<<endl;
    cout<<"CPI:"<<double(cycles)/double(inst_num)<<endl;
	return 0;
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

void IF()
{
	if(print_inst)
	{
		cout<<"IF:"<<PC<<endl;
	}
	IFID.inst = read_mem_4(PC);
	IFID.pc = PC;

	inst_num ++;
    cycles ++;
	PC+=4;
}
void ID()
{
	unsigned int OP=0;
	unsigned int fuc3=0,fuc7=0;
	int rs1=0,rs2=0,rd=0;
	unsigned int imm_I=0;
	unsigned int imm_S=0;
	unsigned int imm_SB=0;
	unsigned int imm_U=0;
	unsigned int imm_UJ=0;
	unsigned int shamt = 0;
    unsigned int shamt_5 = 0;


	unsigned int inst = IFID.inst;
	//int pc = IFID.pc;

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
    shamt_5 = getbit(inst,20,24);

    if(print_inst)
	{
		cout<<"ID:";
	}

	IDEX.step = NONE_STEP;
	IDEX.aluop = NONE_ALUOP;
	IDEX.inst_type = NONE_TYPE;
	IDEX.memop = NONE_MEMOP;
	IDEX.bran = NONE_BRANCH;

    if(OP==OP_R)
	{

        IDEX.inst_type = R;
        IDEX.step = IIEW;
        IDEX.rd = rd;
        IDEX.rs1 = rs1;
        IDEX.rs2 = rs2;

		if(fuc3==F3_ADD&&fuc7==F7_ADD)
		{
            IDEX.aluop = ADD;
            if(print_inst)
            {
                cout<<"add\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
		}
		else if(fuc3==F3_MUL&&fuc7==F7_MUL)
        {
            IDEX.aluop = MUL;
            if(print_inst)
            {
                cout<<"mul\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
		else if(fuc3==F3_SUB&&fuc7==F7_SUB)
		{
            IDEX.aluop = SUB;
            if(print_inst)
            {
                cout<<"sub\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SLL&&fuc7==F7_SLL)
        {
            IDEX.aluop = SLL;
            if(print_inst)
            {
                cout<<"sll\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_MULH&&fuc7==F7_MULH)
        {
            IDEX.aluop = MULH;
            if(print_inst)
            {
                cout<<"mulh\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SLT&&fuc7==F7_SLT)
        {
            IDEX.aluop = SLT;
            if(print_inst)
            {
                cout<<"slt\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_XOR&&fuc7==F7_XOR)
        {
            IDEX.aluop = XOR;
            if(print_inst)
            {
                cout<<"xor\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_DIV&&fuc7==F7_DIV)
        {
            IDEX.aluop = DIV;
            if(print_inst)
            {
                cout<<"div\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SRL&&fuc7==F7_SRL)
        {
            IDEX.aluop = SRL;
            if(print_inst)
            {
                cout<<"srl\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SRA&&fuc7==F7_SRA)
        {
            IDEX.aluop = SRA;
            if(print_inst)
            {
                cout<<"sra\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_OR&&fuc7==F7_OR)
        {
            IDEX.aluop = OR;
            if(print_inst)
            {
                cout<<"or\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_REM&&fuc7==F7_REM)
        {
            IDEX.aluop = REM;
            if(print_inst)
            {
                cout<<"rem\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_AND&&fuc7==F7_AND)
        {
            IDEX.aluop = AND;
            if(print_inst)
            {
                cout<<"and\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else
        {
        	cout<<"other R inst:"<<inst<<endl;
        }
    }
    else if(OP==OP_RW)
    {

    	IDEX.inst_type = RW;
        IDEX.step = IIEW;
        IDEX.rd = rd;
        IDEX.rs1 = rs1;
        IDEX.rs2 = rs2;

        if(fuc3==F3_ADDW&&fuc7==F7_ADDW)
        {
            IDEX.aluop = ADD;
            if(print_inst)
            {
                cout<<"addw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_SUBW&&fuc7==F7_SUBW)
        {
            IDEX.aluop = SUB;
            if(print_inst)
            {
                cout<<"subw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_MULW&&fuc7==F7_MULW)
        {
            IDEX.aluop = MUL;
            if(print_inst)
            {
                cout<<"mulw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_DIVW&&fuc7==F7_DIVW)
        {
            IDEX.aluop = DIV;
            if(print_inst)
            {
                cout<<"divw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else if(fuc3==F3_REMW&&fuc7==F7_REMW)
        {
            IDEX.aluop = REM;
            if(print_inst)
            {
                cout<<"remw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<"x"<<rs2<<endl;
            }
        }
        else
        {
        	cout<<"other RW inst:"<<inst<<endl;
        }
    }
    else if(OP==OP_LW)
    {
    	IDEX.inst_type = LW;
        IDEX.step = IIEMW;
        IDEX.rd = rd;
        IDEX.rs1 = rs1;
        IDEX.memop = LOAD;
        IDEX.imm = imm_I;

        if(fuc3==F3_LB)
        {
            IDEX.memsize = 1;
            if(print_inst)
            {
                cout<<"lb\t"<<"x"<<rd<<","<<ext_signed_64(imm_I,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_LH)
        {
            IDEX.memsize = 2;
            if(print_inst)
            {
                cout<<"lh\t"<<"x"<<rd<<","<<ext_signed_64(imm_I,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_LW)
        {
            IDEX.memsize = 4;
            if(print_inst)
            {
                cout<<"lw\t"<<"x"<<rd<<","<<ext_signed_64(imm_I,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_LD)
        {
            IDEX.memsize = 8;
            if(print_inst)
            {
                cout<<"ld\t"<<"x"<<rd<<","<<ext_signed_64(imm_I,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else
        {
        	cout<<"other LW inst:"<<inst<<endl;
        }
    }
	else if(OP==OP_I)
    {
    	IDEX.inst_type = I;
        IDEX.step = IIEW;
        IDEX.rd = rd;
        IDEX.rs1 = rs1;

        if(fuc3==F3_ADDI)
        {
            IDEX.aluop =ADD;
        	IDEX.imm = imm_I;
            if(print_inst)
            {
                cout<<"addi\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,1,12)<<endl;
            }
        }
        else if(fuc3==F3_SLLI&&fuc7==F7_SLLI)
        {
           IDEX.aluop = SLL;
           IDEX.imm = shamt;
           if(print_inst)
            {
                cout<<"slli\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<shamt<<endl;
            }
        }
        else if(fuc3==F3_SLTI)
        {
            IDEX.aluop =SLT;
        	IDEX.imm = imm_I;
            if(print_inst)
            {
                cout<<"slti\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,1,12)<<endl;
            }
        }
        else if(fuc3==F3_XORI)
        {
            IDEX.aluop =XOR;
        	IDEX.imm = imm_I;
            if(print_inst)
            {
                cout<<"xori\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,0,12)<<endl;
            }
        }
        else if(fuc3==F3_SRLI&&fuc7==F7_SRLI)
        {
            IDEX.aluop =SRL;
        	IDEX.imm = shamt;
        	if(print_inst)
            {
                cout<<"srli\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<shamt<<endl;
            }
        }
        else if(fuc3==F3_SRAI&&fuc7==F7_SRAI)
        {
            IDEX.aluop = SRA;
        	IDEX.imm = shamt;
            if(print_inst)
            {
                cout<<"srai\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<shamt<<endl;
            }
        }
        else if(fuc3==F3_ORI)
        {
            IDEX.aluop = OR;
        	IDEX.imm = imm_I;
            if(print_inst)
            {
                cout<<"ori\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,0,12)<<endl;
            }
        }
        else if(fuc3==F3_ANDI)
        {
            IDEX.aluop = AND;
        	IDEX.imm = imm_I;
            if(print_inst)
            {
                cout<<"andi\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,0,12)<<endl;
            }
        }
        else
        {
        	cout<<"other I inst:"<<inst<<endl;
        }
    }
    else if(OP==OP_IW)
    {
    	IDEX.inst_type = IW;
        IDEX.step = IIEW;
        IDEX.rd = rd;
        IDEX.rs1 = rs1;

        if(fuc3==F3_ADDIW)
        {
            IDEX.aluop = ADD;
        	IDEX.imm = imm_I;
            if(print_inst)
            {
                cout<<"addiw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed(imm_I,1,12)<<endl;
            }
        }
        else if(fuc3==F3_SLLIW & fuc7 == F7_SLLIW)
        {
            IDEX.aluop = SLL;
            IDEX.imm = shamt_5;
            if(print_inst)
            {
                cout<<"slliw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed(shamt_5,1,12)<<endl;
            }
        }
        else if(fuc3==F3_SRLIW & fuc7 == F7_SRLIW)
        {
            IDEX.aluop = SRL;
            IDEX.imm = shamt_5;
            if(print_inst)
            {
                cout<<"srliw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed(shamt_5,1,12)<<endl;
            }
        }
        else if(fuc3==F3_SRAIW & fuc7 == F7_SRAIW)
        {
            IDEX.aluop = SRA;
            IDEX.imm = shamt_5;
            if(print_inst)
            {
                cout<<"sraiw\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed(shamt_5,1,12)<<endl;
            }
        }
        else
        {
        	cout<<"other IW inst:"<<inst<<endl;
        }
    }
    else if(OP==OP_JALR)
    {
        if(fuc3==F3_JALR)
        {
            IDEX.inst_type = JALR;
	        IDEX.step = IIEW;
	        IDEX.rd = rd;
	        IDEX.rs1 = rs1;
	        IDEX.imm = imm_I;
	        IDEX.pc = IFID.pc;
            if(print_inst)
            {
                cout<<"jalr\t"<<"x"<<rd<<","<<"x"<<rs1<<","<<ext_signed_64(imm_I,1,12)<<endl;
            }
        }
        else
        {
        	cout<<"other JALR inst:"<<inst<<endl;
        }
    }
    else if(OP==OP_SCALL)//系统调用指令
    {
        if(fuc3==F3_SCALL&&fuc7==F7_SCALL)
        {
        	if(print_inst)
            {
                cout<<"SCALL"<<endl;
            }
        	IDEX.step = IIE;
        	IDEX.inst_type = SCALL;
        }
        else
        {
        	cout<<"other SCALL inst:"<<inst<<endl;
        }
    }
    else if(OP==OP_SW)
    {
    	IDEX.inst_type = SW;
        IDEX.step = IIEM;
        IDEX.rs1 = rs1;
        IDEX.rs2 = rs2;
        IDEX.memop = STORE;
        IDEX.imm = imm_S;

        if(fuc3==F3_SB)
        {
            IDEX.memsize = 1;
            if(print_inst)
            {
                cout<<"sb\t"<<"x"<<rs2<<","<<ext_signed_64(imm_S,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_SH)
        {
            IDEX.memsize = 2;
            if(print_inst)
            {
                cout<<"sh\t"<<"x"<<rs2<<","<<ext_signed_64(imm_S,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_SW)
        {
            IDEX.memsize = 4;
            if(print_inst)
            {
                cout<<"sw\t"<<"x"<<rs2<<","<<ext_signed_64(imm_S,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_SD)
        {
            IDEX.memsize = 8;
            if(print_inst)
            {
                cout<<"sd\t"<<"x"<<rs2<<","<<ext_signed_64(imm_S,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else
        {
        	cout<<"other SW inst:"<<inst<<endl;
        }
    }
    else if(OP==OP_BRANCH)
    {
    	IDEX.inst_type = BRANCH;
        IDEX.step = IIE;
        IDEX.rs1 = rs1;
        IDEX.rs2 = rs2;
        IDEX.pc = IFID.pc;
        IDEX.imm = imm_SB;

        if(fuc3==F3_BEQ)
        {
        	IDEX.bran = BEQ;
            if(print_inst)
            {
                cout<<"beq\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_BNE)
        {
        	IDEX.bran = BNE;
            if(print_inst)
            {
                cout<<"bne\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_BLT)
        {
        	IDEX.bran = BLT;
            if(print_inst)
            {
                cout<<"blt\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_BGE)
        {
        	IDEX.bran = BGE;
            if(print_inst)
            {
                cout<<"bge\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_BLTU)
        {
            IDEX.bran = BLTU;
            if(print_inst)
            {
                cout<<"bltu\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else if(fuc3==F3_BGEU)
        {
            IDEX.bran = BGEU;
            if(print_inst)
            {
                cout<<"bgeu\t"<<"x"<<rs2<<","<<ext_signed(imm_SB,1,12)<<"("<<"x"<<rs1<<")"<<endl;
            }
        }
        else
        {
        	cout<<"other BRANCH inst:"<<inst<<endl;
        }
    }
    else if(OP==OP_AUIPC)
    {
        if(1){
            IDEX.inst_type = AUIPC;
	        IDEX.step = IIEW;
	        IDEX.rd = rd;
	        IDEX.pc = IFID.pc;
	        IDEX.imm = imm_U;

            if(print_inst)
            {
                cout<<"auipc\t"<<"x"<<rd<<","<<ext_signed_64(imm_U,1,20)<<endl;
            }
        }
    }
    else if(OP==OP_LUI)
    {
        if(1){
        	IDEX.inst_type = LUI;
	        IDEX.step = IIEW;
	        IDEX.rd = rd;
	        IDEX.imm = imm_U;
            if(print_inst)
            {
                cout<<"lui\t"<<"x"<<rd<<","<<ext_signed_64(imm_U,1,20)<<endl;
            }
        }
    }
    else if(OP==OP_JAL)
    {
        if(1){
            IDEX.inst_type = JAL;
	        IDEX.step = IIEW;
	        IDEX.rd = rd;
	        IDEX.imm = imm_UJ;
	        IDEX.pc = IFID.pc;

            if(print_inst)
            {
                cout<<"jal\t"<<"x"<<rd<<","<<ext_signed(imm_UJ,1,20)<<endl;
            }
        }
    }
    else
    {
        cout<<"ERROR inst:"<<inst<<endl;
    }
    cycles ++;
}
void EX()
{
	if(print_inst)
	{
		cout<<"EX:"<<endl;
	}
	if(IDEX.inst_type == R)
	{
		EXMEM.rd = IDEX.rd;
		if(IDEX.aluop == ADD)
		{
			EXMEM.EXout = reg[IDEX.rs1] + reg[IDEX.rs2];
		}
		else if (IDEX.aluop == MUL)
		{
			EXMEM.EXout = reg[IDEX.rs1] * reg[IDEX.rs2];
		}
		else if (IDEX.aluop == SUB)
		{
			EXMEM.EXout = reg[IDEX.rs1] - reg[IDEX.rs2];
		}
		else if (IDEX.aluop == SLL)
		{
			EXMEM.EXout = reg[IDEX.rs1] << reg[IDEX.rs2];
		}
		else if (IDEX.aluop == MULH)
		{
			__uint128_t result = __uint128_t(reg[IDEX.rs1])*__uint128_t(reg[IDEX.rs2]);
            result = result >> 64;
			EXMEM.EXout = result;
		}
		else if (IDEX.aluop == SLT)
		{
			EXMEM.EXout = (reg[IDEX.rs1]<reg[IDEX.rs2])?1:0;
		}
		else if (IDEX.aluop == XOR)
		{
			EXMEM.EXout = reg[IDEX.rs1]^reg[IDEX.rs2];
		}
		else if (IDEX.aluop == DIV)
		{
			EXMEM.EXout = reg[IDEX.rs1]/reg[IDEX.rs2];
		}
		else if (IDEX.aluop == SRL)
		{
			EXMEM.EXout = reg[IDEX.rs1]>>reg[IDEX.rs2];
		}
		else if (IDEX.aluop == SRA)
		{
			long long int reg_rs1_signed = reg[IDEX.rs1];
			EXMEM.EXout = reg_rs1_signed >> reg[IDEX.rs2];
		}
		else if (IDEX.aluop == OR)
		{
			EXMEM.EXout = reg[IDEX.rs1]|reg[IDEX.rs2];
		}
		else if (IDEX.aluop == REM)
		{
			EXMEM.EXout = reg[IDEX.rs1]%reg[IDEX.rs2];
		}
		else if (IDEX.aluop == AND)
		{
			EXMEM.EXout = reg[IDEX.rs1]&reg[IDEX.rs2];
		}
	}
	else if (IDEX.inst_type == RW)
	{
		EXMEM.rd = IDEX.rd;
		if(IDEX.aluop == ADD)
		{
			int reg_rd = (reg[IDEX.rs1]&0xFFFFFFFF) + (reg[IDEX.rs2]&0xFFFFFFFF);
            EXMEM.EXout = ext_signed_64(reg_rd, 1, 32);
		}
		else if(IDEX.aluop == SUB)
		{
			int reg_rd = (reg[IDEX.rs1]&0xFFFFFFFF) - (reg[IDEX.rs2]&0xFFFFFFFF);
            EXMEM.EXout = ext_signed_64(reg_rd, 1, 32);
		}
		else if(IDEX.aluop == MUL)
		{
			int reg_rd = (reg[IDEX.rs1]&0xFFFFFFFF) * (reg[IDEX.rs2]&0xFFFFFFFF);
            EXMEM.EXout = ext_signed_64(reg_rd, 1, 32);
		}
		else if(IDEX.aluop == DIV)
		{
			int reg_rd = (reg[IDEX.rs1]&0xFFFFFFFF) / (reg[IDEX.rs2]&0xFFFFFFFF);
            EXMEM.EXout = ext_signed_64(reg_rd, 1, 32);
		}
		else if(IDEX.aluop == REM)
		{
			int reg_rd = (reg[IDEX.rs1]&0xFFFFFFFF) % (reg[IDEX.rs2]&0xFFFFFFFF);
            EXMEM.EXout = ext_signed_64(reg_rd, 1, 32);
		}
	}
	else if (IDEX.inst_type == LW)
	{
		EXMEM.rd = IDEX.rd;
		EXMEM.memsize = IDEX.memsize;
		EXMEM.memop = IDEX.memop;
		EXMEM.EXout = reg[IDEX.rs1] + ext_signed_64(IDEX.imm,1,12);
	}
	else if(IDEX.inst_type == I)
	{
		EXMEM.rd = IDEX.rd;
		if(IDEX.aluop == ADD)
		{
			EXMEM.EXout = reg[IDEX.rs1] + ext_signed_64(IDEX.imm,1,12);
		}
		else if(IDEX.aluop == SLL)
		{
			EXMEM.EXout = reg[IDEX.rs1] << IDEX.imm;
		}
		else if(IDEX.aluop == SLT)
		{
			EXMEM.EXout = (reg[IDEX.rs1] < ext_signed_64(IDEX.imm,1,12))?1:0;
		}
		else if(IDEX.aluop == XOR)
		{
			EXMEM.EXout = reg[IDEX.rs1] ^ ext_signed_64(IDEX.imm,0,12);
		}
		else if(IDEX.aluop == SRL)
		{
			EXMEM.EXout = reg[IDEX.rs1] >> IDEX.imm;
		}
		else if(IDEX.aluop == SRA)
		{
			long long int reg_rs1_signed = reg[IDEX.rs1];
			EXMEM.EXout = reg_rs1_signed >> IDEX.imm;
		}
		else if(IDEX.aluop == OR)
		{
			EXMEM.EXout = reg[IDEX.rs1] | ext_signed_64(IDEX.imm,0,12);
		}
		else if(IDEX.aluop == AND)
		{
			EXMEM.EXout = reg[IDEX.rs1] & ext_signed_64(IDEX.imm,0,12);
		}
	}
	else if(IDEX.inst_type == IW)
	{
		EXMEM.rd = IDEX.rd;
		if(IDEX.aluop == ADD)
		{
			int reg_rd = (reg[IDEX.rs1]&0xFFFFFFFF) + ext_signed(IDEX.imm,1,12);
			EXMEM.EXout = ext_signed_64(reg_rd, 1, 32);
		}
        else if(IDEX.aluop == SLL)
        {
            unsigned int reg_rd = (reg[IDEX.rs1]&0xFFFFFFFF) << IDEX.imm;
            EXMEM.EXout = ext_signed_64(reg_rd, 1, 32);
        }
        else if(IDEX.aluop == SRL)
        {
            unsigned int reg_rd = (reg[IDEX.rs1]&0xFFFFFFFF) >> IDEX.imm;
            EXMEM.EXout = ext_signed_64(reg_rd, 1, 32);
        }
        else if(IDEX.aluop == SRA)
        {
            long long int reg_rs1_signed = reg[IDEX.rs1];
            unsigned int reg_rd = (reg_rs1_signed &0xFFFFFFFF) >> IDEX.imm;
            EXMEM.EXout = ext_signed_64(reg_rd, 1, 32);
        }
	}
	else if(IDEX.inst_type == JALR)
	{
		EXMEM.rd = IDEX.rd;
		EXMEM.EXout = IDEX.pc + 4;
		PC = (reg[IDEX.rs1] + ext_signed_64(IDEX.imm,0,12)) &(-2);
	}
	else if(IDEX.inst_type == SCALL)
	{
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
	else if (IDEX.inst_type == SW)
	{
		EXMEM.rs2 = IDEX.rs2;
		EXMEM.memsize = IDEX.memsize;
		EXMEM.memop = IDEX.memop;
		EXMEM.EXout = reg[IDEX.rs1] + ext_signed_64(IDEX.imm,1,12);
	}
	else if (IDEX.inst_type == BRANCH)
	{
		if(IDEX.bran == BEQ)
		{
			if(reg[IDEX.rs1] == reg[IDEX.rs2])
            {
                PC = IDEX.pc + (ext_signed(IDEX.imm,1,12)<<1);
            }
		}
		else if(IDEX.bran == BNE)
		{
			if(reg[IDEX.rs1] != reg[IDEX.rs2])
            {
                PC = IDEX.pc + (ext_signed(IDEX.imm,1,12)<<1);
            }
		}
		else if(IDEX.bran == BLT)
		{
			if((long long int)(reg[IDEX.rs1]) < (long long int)(reg[IDEX.rs2]))
            {
                PC = IDEX.pc + (ext_signed(IDEX.imm,1,12)<<1);
            }
		}
		else if(IDEX.bran == BGE)
		{
			if((long long int)(reg[IDEX.rs1]) >= (long long int)(reg[IDEX.rs2]))
            {
                PC = IDEX.pc + (ext_signed(IDEX.imm,1,12)<<1);
            }
		}
        else if(IDEX.bran == BLTU)
        {
            if(reg[IDEX.rs1] < reg[IDEX.rs2])
            {
                PC = IDEX.pc + (ext_signed(IDEX.imm,1,12)<<1);
            }
        }
        else if(IDEX.bran == BGEU)
        {
            if(reg[IDEX.rs1] >= reg[IDEX.rs2])
            {
                PC = IDEX.pc + (ext_signed(IDEX.imm,1,12)<<1);
            }
        }

	}
	else if(IDEX.inst_type == AUIPC)
	{
		EXMEM.rd = IDEX.rd;
		EXMEM.EXout = IDEX.pc + (ext_signed_64(IDEX.imm,1,20)<<12);
	}
	else if(IDEX.inst_type == LUI)
	{
		EXMEM.rd = IDEX.rd;
		EXMEM.EXout = ext_signed_64(IDEX.imm,1,20)<<12;
	}
	else if(IDEX.inst_type == JAL)
	{
		EXMEM.rd = IDEX.rd;
		EXMEM.EXout = IDEX.pc + 4;
		PC = IDEX.pc + (ext_signed(IDEX.imm,1,20) << 1);
	}
//!!!!!!
    if(IDEX.aluop == MUL || IDEX.aluop == MULH)
    {
        if(IDEX.inst_type == R){
            cycles += 2;
        }
        else if(IDEX.inst_type == RW){
            cycles += 1;
        }
    }
    else if(IDEX.aluop == DIV){
        cycles += 40;
    }
    else if(IDEX.aluop == REM){
        if(preinst.aluop == DIV && preinst.inst_type == IDEX.inst_type && preinst.rd != preinst.rs1 && preinst.rd != preinst.rs2 && preinst.rs1 == IDEX.rs1 && preinst.rs2 == IDEX.rs2){
            cycles += 0;
        }
        else{
            cycles += 40;
        }
    }
    else{
        cycles++;
    }
    preinst.aluop = IDEX.aluop;
    preinst.inst_type = IDEX.inst_type;
    preinst.rd = IDEX.rd;
    preinst.rs1 = IDEX.rs1;
    preinst.rs2 = IDEX.rs2;
//!!!!!!!!!
}
void MEM()
{
	if(print_inst)
	{
		cout<<"MEM"<<endl;
	}
	if(EXMEM.memop == LOAD)
	{
		MEMWB.rd = EXMEM.rd;
		if(EXMEM.memsize == 1)
		{
			MEMWB.MEMout = ext_signed_64(memory[EXMEM.EXout], 1, 8);
		}
		if(EXMEM.memsize == 2)
		{
			MEMWB.MEMout = ext_signed_64(read_mem_2(EXMEM.EXout), 1, 16);
		}
		if(EXMEM.memsize == 4)
		{
			MEMWB.MEMout = ext_signed_64(read_mem_4(EXMEM.EXout), 1, 32);
		}
		if(EXMEM.memsize == 8)
		{
			MEMWB.MEMout = read_mem_8(EXMEM.EXout);
		}
	}
	else if(EXMEM.memop == STORE)
	{
		if(EXMEM.memsize == 1)
		{
			memory[EXMEM.EXout] = (unsigned char)reg[EXMEM.rs2];
		}
		if(EXMEM.memsize == 2)
		{
			write_mem_2(EXMEM.EXout, (unsigned short)reg[EXMEM.rs2]);
		}
		if(EXMEM.memsize == 4)
		{
			write_mem_4(EXMEM.EXout, (unsigned int)reg[EXMEM.rs2]);
		}
		if(EXMEM.memsize == 8)
		{
			write_mem_8(EXMEM.EXout, reg[EXMEM.rs2]);
		}
	}
    cycles ++;
}
void WB()
{
	if(print_inst)
	{
		cout<<"WB:"<<endl;
	}
	reg[MEMWB.rd] = MEMWB.MEMout;
    cycles++;
}