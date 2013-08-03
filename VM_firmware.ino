/************************************************************
  Author :                      Nakul Rao I
  Date began :                  26/7/2013
  Date completed :              1/8/2013
  Instruction set verified on : 3/8/2013
 ************************************************************/

 #include <stdint.h>
 #include <avr/interrupt.h>
 #include <avr/io.h>
 #include <avr/pgmspace.h>

 #define ROM_SIZE       2048
 #define STACK_SIZE     512
 #define COMMON         64

 // function prototypes
 void clr_VM(byte n);
 void debug(byte n);
 void VM_init(void);
 void load_prog(void);
 void init_prog(byte n);
 void serial_begin(void);
 void run(byte n);
 void reset_VM(void);
 
 // Global Variables
 byte N, i, temp = 1;
 byte opcode, operand;
 int count = 0, com_mem_p = 0; 
 int com_mem[COMMON];

 // VM structure
 struct Virtual_Machine{
   byte prog_mem[ROM_SIZE];           // program memory
   int stack[STACK_SIZE];             // stack memory
   int ip, sp;                   // instruction pointer & stack pointer
   byte op_ready;                // Output ready flag
   byte done;                    // VM execution completed
  };
  
  Virtual_Machine VM[2];      // Declare 2 virtual machines
 
 /****************************************************************************************************************************/
 
 void setup(){
   // begin setup
  pinMode(48, INPUT_PULLUP);      // VM prog mode
  pinMode(49, INPUT_PULLUP);      // VM reset
  
  DDRA = 0x00;      // Input
  DDRB = 0x00;      // Input
  DDRC = 0xFF;      // Output
  DDRD = 0xFF;      // Output

  serial_begin();  
    
  if((N > 2) || (N < 1)){
     Serial.println("VM aborted");
     Serial.println("Reset to start again");
     while(1);
  }
 
   VM_init(); 
  
   // Program VM
   init_prog(N);
   
   Serial.print("Beginning execution in 1 second");
   delay(1000);
 }                              // setup completed

 /****************************************************************************************************************************************/
 
 void loop(){
   if (digitalRead(48) == LOW){
     load_prog();
   }
   
   if (digitalRead(49) == LOW){
     reset_VM();
   }
   
   // Run VM1
   if ((VM[0].done == 0)){
     run(0);
   }
      
   // Run VM2
   if((N > 1) && (VM[1].done == 0)){   
     run(1);
   }     

 }

/*****************************************************************************************************************************************/

// clears the program memory and stack of the VM
 void clr_VM(byte n){
   int count;
   for(count = 0; count < ROM_SIZE; count++){
     VM[n].prog_mem[count] = 0;
   }
   for(count = 0; count < STACK_SIZE; count++){
      VM[n].stack[count] = 0;
   }   
 }

 // prints all the parameters of the VM
 void debug(byte n){
   Serial.println("\n Instruction Pointer : ");
   Serial.println(VM[n].ip);
   Serial.println("\n Stack Pointer : ");
   Serial.println(VM[n].sp);
   Serial.println("\n Top of Stack : ");
   Serial.println(VM[n].stack[VM[n].sp]);
   Serial.println("\n Output Ready Flag : ");
   Serial.println(VM[n].op_ready);
   Serial.println("\n Execution Done Flag : ");
   Serial.println(VM[n].done);
 }
 
 // initialize the VMs
 void VM_init(void){
    for(i = 0; i < (N); i++){
    VM[i].ip = 0;
    VM[i].sp = 0;
    VM[i].op_ready = 0;
    VM[i].done = 0;
    clr_VM(i);
  }
 }
 
 // reset the VM
 void reset_VM(void){
   byte n;
   int count;
   Serial.print("Which VM do you want to reset?");
   while(Serial.available() == 0){};
   if (Serial.available() > 0) {
      n = Serial.read();
   }
   VM[n].ip = 0;
   VM[n].sp = 0;
   VM[n].op_ready = 0;
   VM[n].done = 0;
   for(count = 0; count < STACK_SIZE; count++){
     VM[n].stack[count] = 0;
   }
 }   

 // load program for the VM
 void load_prog(void){
   byte n;
   count = 0; 
   Serial.println("Which VM do you want to program? ");
   while(Serial.available() == 0){};
   if (Serial.available() > 0) {
      n = Serial.read();
   }
   if (n <= N){
     Serial.println("Enter opcodes ");
     while(temp != 255){
      if (Serial.available() > 0) {
        temp = Serial.read();
        VM[n-1].prog_mem[count] = temp;
        count++;
     }
    }
   }
   else{
     Serial.println(" VM not available ");
     Serial.println(" Please reset the firmware ");
     while(1){}
   }
 }
 
 void init_prog(byte n){
   Serial.println("Program VM1");
   while(temp != 255){
      if (Serial.available() > 0) {
        temp = Serial.read();
        VM[0].prog_mem[count] = temp;
        count++;
     }
    }
    if(n > 1){
      temp = 0;
      Serial.println("Program VM2");
      while(temp != 255){
        if (Serial.available() > 0) {
          temp = Serial.read();
          VM[1].prog_mem[count] = temp;
          count++;
       }
     }
    }
 }
 
 void serial_begin(void){
  Serial.begin(9600);                                                        // Init
  Serial.println("Welcome to Virtual Machine");
  Serial.println("The device can run a maximum of 3 virtual machines");
  Serial.println("How many virtual machines do you want to run?");
  while(Serial.available() == 0){};
  N = Serial.read();
 }
 
 void run(byte n){
   
   switch(VM[n].prog_mem[VM[n].ip]){
       case 0:
         // NOP
         VM[n].ip++;
         break;

       case 1:
         // reap |||||   read port value into stack
         VM[n].ip++;
         VM[n].stack[STACK_SIZE-4] = PORTA;
         VM[n].stack[STACK_SIZE-3] = PORTB;         
         break;

       case 2:
         // wrip P ||||| write stack values into the port
         VM[n].ip++;
         PORTC = VM[n].stack[STACK_SIZE-2];
         PORTD = VM[n].stack[STACK_SIZE-1];
         break;

       case 3:
         // jmpf A ||||| ip = ip + A (relative jump) forward jump only
         VM[n].ip++;
         VM[n].ip = VM[n].ip + ((VM[n].prog_mem[VM[n].ip] * 100) + (VM[n].prog_mem[VM[n].ip + 1]));
         VM[n].ip++;
         VM[n].ip++;
         break;
     
       case 4:
         // jmpb A ||||| ip = ip - A (relative jump) backward jump only
         VM[n].ip++;
         VM[n].ip = VM[n].ip - ((VM[n].prog_mem[VM[n].ip] * 100) + (VM[n].prog_mem[VM[n].ip + 1]));
         VM[n].ip++;
         VM[n].ip++;
         break;
       
       case 5:
         // jfz ||||| if top of stack is 0, ip = ip + A (forward jump)
         VM[n].ip++;
         if(VM[n].stack[VM[n].sp] == 0){
           VM[n].ip = VM[n].ip + ((VM[n].prog_mem[VM[n].ip] * 100) + (VM[n].prog_mem[VM[n].ip + 1]));
           VM[n].ip++;
           VM[n].ip++;
         }
         else{
           VM[n].ip++;
           VM[n].ip++;
         }
         break;
         
       case 6:
         // jbz ||||| if top of stack is 0, ip = ip - A (backward jump)
         VM[n].ip++;
         if(VM[n].stack[VM[n].sp] == 0){
           VM[n].ip = VM[n].ip - ((VM[n].prog_mem[VM[n].ip] * 100) + (VM[n].prog_mem[VM[n].ip + 1]));
           VM[n].ip++;
           VM[n].ip++;
         }
         else{
           VM[n].ip++;
           VM[n].ip++;
         }
         break;
       
       case 7:
         // jfnz ||||| if top of stack not 0, ip = ip + A (forward jump)
         VM[n].ip++;
         if (VM[n].stack[VM[n].sp] != 0){
           VM[n].ip = VM[n].ip + ((VM[n].prog_mem[VM[n].ip] * 100) + (VM[n].prog_mem[VM[n].ip + 1]));
           VM[n].ip++;
           VM[n].ip++;
         }
         else{
           VM[n].ip++;
           VM[n].ip++;
         }
         break;
       
       case 8:
         // jbnz ||||| if top of stack not 0, ip = ip - A (backward jump)
         VM[n].ip++;
         if (VM[n].stack[VM[n].sp] != 0){
           VM[n].ip = VM[n].ip - ((VM[n].prog_mem[VM[n].ip] * 100) + (VM[n].prog_mem[VM[n].ip + 1]));
           VM[n].ip++;
           VM[n].ip++;
         }
         else{
           VM[n].ip++;
           VM[n].ip++;
         }
         break;
        
       case 9:
         // load ||||| top of stack = stack[content of top of stack]
         VM[n].ip++;
         VM[n].sp++;
         VM[n].stack[VM[n].sp] = VM[n].stack[VM[n].stack[VM[n].sp]];
         break;
        
       case 10:
         // store ||||| stack[content of (top of stack - 1)] = top of stack
         VM[n].ip++;
         VM[n].stack[VM[n].stack[VM[n].sp - 1]] = VM[n].stack[VM[n].sp];
         break;
       
       case 11:
         // add ||||| top of stack = top of stack + second of stack 
         VM[n].ip++;
         VM[n].stack[VM[n].sp - 1] = VM[n].stack[VM[n].sp] + VM[n].stack[VM[n].sp - 1];
         VM[n].sp--;       
         break;
       
       case 12:
         // sub ||||| top of stack = second of stack - top of stack
         VM[n].ip++;
         VM[n].stack[VM[n].sp - 1] = VM[n].stack[VM[n].sp - 1] - VM[n].stack[VM[n].sp];
         VM[n].sp--;
         break;
       
       case 13:
         // mul ||||| top of stack = top of stack * second of stack
         VM[n].ip++;
         VM[n].stack[VM[n].sp - 1] = VM[n].stack[VM[n].sp] * VM[n].stack[VM[n].sp - 1];
         VM[n].sp--;       
         break;
        
       case 14:
         // and ||||| top of stack = top of stack && second of stack
         VM[n].ip++;
         VM[n].stack[VM[n].sp - 1] = VM[n].stack[VM[n].sp] && VM[n].stack[VM[n].sp - 1];
         VM[n].sp--;       
         break;
       
       case 15:
         // or ||||| top of stack = top of stack || second of stack
         VM[n].ip++;
         VM[n].stack[VM[n].sp - 1] = VM[n].stack[VM[n].sp] || VM[n].stack[VM[n].sp - 1];
         VM[n].sp--;       
         break;
         
       case 16:
         // band ||||| top of stack = top of stack & second of stack
         VM[n].ip++;
         VM[n].stack[VM[n].sp - 1] = VM[n].stack[VM[n].sp] & VM[n].stack[VM[n].sp - 1];
         VM[n].sp--;       
         break;

       case 17:
         // bor ||||| top of stack = top of stack | second of stack
         VM[n].ip++;
         VM[n].stack[VM[n].sp - 1] = VM[n].stack[VM[n].sp] | VM[n].stack[VM[n].sp - 1];
         VM[n].sp--;       
         break;
       
       case 18:
         // bxor ||||| top of stack = top of stack ^ second of stack
         VM[n].ip++;
         VM[n].stack[VM[n].sp - 1] = VM[n].stack[VM[n].sp] ^ VM[n].stack[VM[n].sp - 1];
         VM[n].sp--;       
         break;
       
        case 19:
          // eq ||||| is top of stack == second of stack?
          VM[n].ip++;
          if (VM[n].stack[VM[n].sp] == VM[n].stack[VM[n].sp - 1]) {
            VM[n].stack[VM[n].sp - 1] = 1;
          }
          else{
            VM[n].stack[VM[n].sp - 1] = 0;
          }
          VM[n].sp--;
          break;
          
        case 20:
          // gt ||||| is second of stack > top of stack?
          VM[n].ip++;
          if (VM[n].stack[VM[n].sp - 1] > VM[n].stack[VM[n].sp]) {
            VM[n].stack[VM[n].sp - 1] = 1;
          }
          else{
            VM[n].stack[VM[n].sp - 1] = 0;
          }
          VM[n].sp--;
          break;
       
        case 21:
          // lt ||||| is second of stack < top of stack?
          VM[n].ip++;
          if (VM[n].stack[VM[n].sp - 1] < VM[n].stack[VM[n].sp]) {
            VM[n].stack[VM[n].sp - 1] = 1;
          }
          else{
            VM[n].stack[VM[n].sp - 1] = 0;
          }
          VM[n].sp--;
          break;
        
        case 22:
          // ge ||||| is second of stack >= top of stack?
          VM[n].ip++;
          if (VM[n].stack[VM[n].sp - 1] >= VM[n].stack[VM[n].sp]) {
            VM[n].stack[VM[n].sp - 1] = 1;
          }
          else{
            VM[n].stack[VM[n].sp - 1] = 0;
          }
          VM[n].sp--;
          break;

        case 23:
          // le ||||| is second of stack <= top of stack?
          VM[n].ip++;
          if (VM[n].stack[VM[n].sp - 1] <= VM[n].stack[VM[n].sp]) {
            VM[n].stack[VM[n].sp - 1] = 1;
          }
          else{
            VM[n].stack[VM[n].sp - 1] = 0;
          }
          VM[n].sp--;
          break;

        case 24:
          // ne ||||| is second of stack != top of stack?
          VM[n].ip++;
          if (VM[n].stack[VM[n].sp - 1] != VM[n].stack[VM[n].sp]) {
            VM[n].stack[VM[n].sp - 1] = 1;
          }
          else{
            VM[n].stack[VM[n].sp - 1] = 0;
          }
          VM[n].sp--;
          break;
        
        case 25:
          // drop ||||| drop the top of stack
          VM[n].ip++;
          VM[n].stack[VM[n].sp] = 0;
          VM[n].sp--;
          break;
        
        case 26:
          // dup ||||| duplicate top of stack on top of stack
          VM[n].ip++;
          VM[n].stack[VM[n].sp + 1] = VM[n].stack[VM[n].sp];
          VM[n].sp++;
          break;
       
        case 27:
          // over ||||| add second of stack on top of stack
          VM[n].ip++;
          VM[n].stack[VM[n].sp + 1] = VM[n].stack[VM[n].sp - 1];
          VM[n].sp++;
          break;
        
        case 28:
          //rs ||||| right shift the top of stack
          VM[n].ip++;
          VM[n].stack[VM[n].sp] >> 1;
          break;
        
        case 29:
          // ls ||||| left shift the top of stack
          VM[n].ip++;
          VM[n].stack[VM[n].sp] << 1;
          break;
        
        case 30:
          // ed ||||| execution done
          VM[n].done = 1;
          debug(n);
          VM[n].ip = n;
          VM[n].sp = n;
          break;
          
        case 31:
          // sero ||||| top of stack is sent via uart
          VM[n].ip++;
          Serial.print("VM");
          Serial.print(n);
          Serial.print(VM[n].stack[VM[n].sp]);
          VM[n].stack[VM[n].sp] = 0;
          VM[n].sp--;
          break;
          
        case 32:
          // put A |||| put A to the top of stack
          VM[n].ip++;
          VM[n].sp++;
          VM[n].stack[VM[n].sp] = (VM[n].prog_mem[VM[n].ip] * 100) + VM[n].prog_mem[VM[n].ip + 1];
          VM[n].ip++;
          VM[n].ip++;
          break;
          
        case 33:
          // opr ||||| output ready; used only if multiple VMs are running one after another
          VM[n].ip++;
          VM[n].op_ready = 1;
          break;
          
        case 34:
          // pushc ||||| Push the top of stack into top of common memory
          VM[n].ip++;
          com_mem_p++;
          com_mem[com_mem_p] = VM[n].stack[VM[n].sp];
          break;
          
        case 35:
          // popc ||||| Pop the top of common memory to the top of stack
          VM[n].ip++;
          VM[n].sp++;
          VM[n].stack[VM[n].sp] = com_mem[com_mem_p];
          com_mem_p--;
          break;
          
        case 36:
          //wopr B ||||| Wait for VM B’s output to be ready
          if (VM[VM[n].prog_mem[VM[n].ip + 1] - 1].op_ready == 1){
            VM[VM[n].prog_mem[VM[n].ip + 1] - 1].op_ready = 0;
            VM[n].ip++;
            VM[n].ip++;
          }
          else 
            Serial.println("Waiting");
          break;
          
        default:
          Serial.println(VM[n].prog_mem[VM[n].ip]);
          Serial.println(" Error in OPCODE ");
          Serial.println(" All VMs aborted");
          debug(n);
          while(1){}
     }
 }
