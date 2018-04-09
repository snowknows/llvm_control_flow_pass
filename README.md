# llvm_control_flow_pass
示例 a2.c：    
![a2.c](https://github.com/snowknows/llvm_control_flow_pass/blob/master/pic/a2.png)    
     
示例 a1.h:    
![a1.h](https://github.com/snowknows/llvm_control_flow_pass/blob/master/pic/a2.png)    
    
opt -load ./control_flow_analysis.so -cfg a2.bc:    
![a2_control_flow](https://github.com/snowknows/llvm_control_flow_pass/blob/master/pic/a2_cf.png)
    
python generate_cfg.py a2_controlflow.txt:    
![a2_cfg](https://github.com/snowknows/llvm_control_flow_pass/blob/master/pic/a2_controlflow.png)    
![a2_dl](https://github.com/snowknows/llvm_control_flow_pass/blob/master/pic/a2_dl.png)    
