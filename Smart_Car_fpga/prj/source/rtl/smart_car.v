/*******************************************************************************/
/*                                                                             */
/*                               С��fpga����ģ��                                */
/*                                    ��ͩ��                                    */
/*                                  2023/7/14                                  */
/*                                                                             */  
/*******************************************************************************/
module smart_car(
    input               sys_clk  ,              //ϵͳʱ��
    input               sys_rst_n,              //ϵͳ��λ���͵�ƽ��Ч
    input               echo_a,                 //�����ź�echo_a
    input               echo_b,                 //�����ź�echo_b
    input               echo_en_n,              //�������ʹ�ܣ��͵�ƽ��Ч
    input               a,b,                    //С�������ź�
    input               p1,p2,p3,               //��������ź�
    input               en_ab,                  //С���˶�ʹ��
    output  reg         rgb_p,                  //led�����ź� 
    output  reg         pwm_ab,                 //С���˶�pwm
    output  reg         pwm1,pwm2,pwm3,pwm4,    //��������ź�
    output  reg         l1,l2,l3,l4,            //С�������ź�
    output  reg         trig,                   //�����ź�echo_a,echo_b����
    output  reg         nr_a,nr_a_led,
    output  reg         nr_b,nr_b_led           //nr_a,nr_b���ҽ���С��30cmΪ��
    );

/***********************************LED����ģ��***********************************/

reg [23:0] reg_rgb; 
reg [ 5:0] counter_rgb;
reg [23:0] counter_rgb_t;
reg [ 4:0] i;
reg [ 2:0] state_rgb;

always @(posedge sys_clk or negedge sys_rst_n) begin          //��λ�����ڼ�����һ����120ns
    if(!sys_rst_n)	begin
        counter_rgb <= 6'b0;
    end
    else if(counter_rgb < 6'd60 - 1'b1)
        counter_rgb <= counter_rgb + 1'b1;
    else begin
        counter_rgb <= 6'd0;
    end
end

always @(posedge sys_clk or negedge sys_rst_n) begin
    if(!sys_rst_n)begin
        state_rgb <= 3'd0;
        counter_rgb_t<=24'd0;
        i <= 5'd0;
    end
    else begin
        if (counter_rgb_t < 1721 & counter_rgb == 59) begin   //������������
            counter_rgb_t <= counter_rgb_t + 1'b1;
            i <= i + 1'b1;
        end
        case(state_rgb)
        3'd0:begin
            reg_rgb <= 24'b00000000_11111111_00000000;        //grb ��ɫ
            if(echo_en_n)begin                                //ͣ��������ʱΪ��ɫ
                state_rgb <= 3'd1;
                counter_rgb_t <= 24'd0;
                end
            else if(en_ab & (!a)&(!b))begin
                state_rgb <= 3'd2;
                counter_rgb_t <= 24'd0;
                end
            end
        3'd1:begin
            if(echo_en_n)
                reg_rgb <= 24'b11111111_11111111_00000000;    //grb ��ɫ
            else if(en_ab & (!a)&(!b))begin                   //��Ѱ�����Լ�����ʱΪ��ɫ  
                state_rgb <= 3'd2; 
                counter_rgb_t <= 24'd0;  
                end
            else if(!(en_ab & (!a)&(!b)))begin   
                state_rgb <= 3'd0;
                counter_rgb_t <= 24'd0;
                end
            end
        3'd2:begin
            reg_rgb <= 24'b11111111_00000000_00000000;        //grb ��ɫ
            if(echo_en_n)begin                                //����ֱ��ʱΪ��ɫ
                state_rgb <= 3'd1; 
                counter_rgb_t <= 24'd0;
                end 
            else if(!(en_ab & (!a)&(!b)))begin   
                state_rgb <= 3'd0;
                counter_rgb_t <= 24'd0; 
                end
             
            end
        endcase
    end 
    if(i == 5'd24 | counter_rgb_t < 1000)
        i <= 0;
end

always @(sys_clk or sys_rst_n or i) begin
    if (counter_rgb_t < 1000) begin
        rgb_p <= 0;
    end
    else if(counter_rgb_t > 999 & counter_rgb_t < 1720) begin
        if (!reg_rgb[5'd23 - i]) begin                         //��λ��Ϊ0���ź�
            if (counter_rgb < 6'd15 - 1'b1)
                rgb_p <= 1;
            else
                rgb_p <= 0;
        end
        else begin
            if (counter_rgb < 6'd30- 1'b1)                     //��λ��Ϊ1���ź�
                rgb_p <= 1;
            else
                rgb_p <=0;
        end
    end
    else begin
        rgb_p <= 1;
    end
end


/***********************************�������ģ��***********************************/

reg [19:0] counter_p;      
                                                                                                                                                                
//pwm_state�仯ʱ����������ϵͳʱ�Ӽ�������ʱ20ms
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n)
        counter_p <= 20'd0;
    else if (counter_p < 20'd1000000 - 1'd1)
        counter_p <= counter_p + 1'b1;
    else
        counter_p <= 20'd0;
end

always @(sys_clk, p1, p2, p3)begin     //p1����צ��, p2,p3���ƻ�е��
//pwm1
    if(!p1)
        begin
                if(counter_p<20'd45000)
                    pwm1<=1'b1;
                else
                    pwm1<=1'b0;        //צ�Ӻ�
        end
    else
        begin
                if(counter_p<20'd25000)
                    pwm1<=1'b1;
                else
                    pwm1<=1'b0;        //צ�ӿ�
        end
//pwm2
    if((!p2) & (!p3))
        begin
                if(counter_p<20'd85000)
                    pwm2<=1'b1;
                else
                    pwm2<=1'b0;        //������
        end
    else
        begin
                if(counter_p<20'd25000)
                    pwm2<=1'b1;
                else
                    pwm2<=1'b0;        //������
        end 
//pwm3
    if((!p2) & (!p3))
        begin
                if(counter_p<20'd85000)
                    pwm3<=1'b1;
                else
                    pwm3<=1'b0;        //С�����
        end
    else 
        begin
                if(counter_p<20'd125000)
                    pwm3<=1'b1;
                else
                    pwm3<=1'b0;        //С�����
        end
//pwm4
    if(p2 & (!p3))
        begin
                if(counter_p<20'd57000)
                    pwm4<=1'b1;
                else
                    pwm4<=1'b0;        //��������
        end
    else if((!p2) & p3)
        begin
                if(counter_p<20'd45000)
                    pwm4<=1'b1;
                else
                    pwm4<=1'b0;        //��������
        end
    else
        begin
                if(counter_p<20'd125000)
                    pwm4<=1'b1;
                else
                    pwm4<=1'b0;        //��������
        end
end
/***********************************С������ģ��***********************************/

//ab��00ǰ  01��  11��  10��

    reg s1,s2,s3;

    always@(a,b)
    begin
        s1=(!a)^(!b);
        s2=!(s1);
        s3=b;
        l1=!(s3);
        l2=b;
        l3=s2;
        l4=s1;
    end

    always@(posedge sys_clk or posedge en_ab)begin
        if(en_ab)begin
            if(counter_p<20'd800000-1'd1)
                pwm_ab<=1'b1;
            else
                pwm_ab<=1'b0;
            end
        else
            pwm_ab<=1'b0;
    end

/************************************trig ģ��************************************/

reg [20:0] counter_t;                            //trig�����ڼ���       
                                                                                                                                                                          
//��������ϵͳʱ�Ӽ�������ʱ30ms
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n)
        counter_t <= 21'd0;
    else if (counter_t < 21'd1500000 - 1'd1)
        counter_t <= counter_t + 1'b1;
    else
        counter_t <= 21'd0;
end

//trig�ź�����30ms������ǰ15usΪ�ߵ�ƽ������Ϊ�͵�ƽ
always @(posedge sys_clk or negedge sys_rst_n) begin
    if (!sys_rst_n)
        trig <= 1'b0;
    else if(counter_t < 21'd750 - 1'd1) 
        trig <= 1'b1;
    else
        trig <= 1'b0;
end

/************************************echo_a ģ��************************************/

reg echo_a2,echo_a1;
assign pose_echo_a =(~echo_a2)&&echo_a1;         //echo_a �����½���
assign nege_echo_a = echo_a2&&(~echo_a1);
parameter aS0 = 2'b00, aS1 = 2'b01, aS2 = 2'b10; // echo_a ״̬���� 
                                                 // aS0:����
                                                 // aS1:��ʼ������
                                                 // aS2:����������
reg[1:0] curr_state_a, next_state_a;
reg [20:0]    counter_e_a;                       //echo_a�ĸߵ�ƽ����
reg [20:0]    dis_reg_a;                         //����echo_a�ߵ�ƽ����ֵ

always@(posedge sys_clk, negedge sys_rst_n)
begin
    if(!sys_rst_n)
    begin
        echo_a1 <= 0;
        echo_a2 <= 0;
        counter_e_a <= 0;
        dis_reg_a <= 0;
        curr_state_a <= aS0;
    end
    else
    begin
        echo_a1 <= echo_a;                        // ��ǰ
        echo_a2 <= echo_a1;                       // ��һ��
        case(curr_state_a)
        aS0:begin
                if (pose_echo_a)                  // ��⵽������
                    curr_state_a <= next_state_a; //aS1
                else
                    counter_e_a <= 21'd0;
            end
        aS1:begin
                if (nege_echo_a)                  // ��⵽�½���
                    curr_state_a <= next_state_a; //aS2
                else
                    begin
                        counter_e_a <= counter_e_a + 1'd1;
                    end
            end
        aS2:begin
                dis_reg_a <= counter_e_a;         // ����������
                counter_e_a <= 0;
                curr_state_a <= next_state_a;     //aS0
            end
        endcase
    end
end

always@(curr_state_a)
begin
    case(curr_state_a)
    aS0:next_state_a <= aS1;
    aS1:next_state_a <= aS2;
    aS2:next_state_a <= aS0;
    endcase
end
//���ҽ�������С��30cm��nr_aΪ��
always@(posedge sys_clk)begin
    if(!echo_en_n) begin
        if(dis_reg_a < 21'd87000 -1'd1)begin
            nr_a<=1'b1;
            nr_a_led<=nr_a;
        end
        else begin
            nr_a<=1'b0;
            nr_a_led<=nr_a;
        end
    end
end
/************************************echo_b ģ��************************************/
reg echo_b2,echo_b1;
assign pose_echo_b =(~echo_b2)&&echo_b1;         //echo_b �����½���
assign nege_echo_b = echo_b2&&(~echo_b1);
parameter bS0 = 2'b00, bS1 = 2'b01, bS2 = 2'b10; // echo_b ״̬���� 
                                                 //bS0:����
                                                 //bS1:��ʼ������ 
                                                 //bS2:����������
reg[1:0] curr_state_b, next_state_b;
reg [20:0]    counter_e_b;                       //echo_b�ĸߵ�ƽ����
reg [20:0]    dis_reg_b;                         //����echo_b�ߵ�ƽ����ֵ

always@(posedge sys_clk, negedge sys_rst_n)
begin
    if(!sys_rst_n)
    begin
        echo_b1 <= 0;
        echo_b2 <= 0;
        counter_e_b <= 0;
        dis_reg_b <= 0;
        curr_state_b <= bS0;
    end
    else
    begin
        echo_b1 <= echo_b;                        // ��ǰ
        echo_b2 <= echo_b1;                       // ��һ��
        case(curr_state_b)
        bS0:begin
                if (pose_echo_b)                  // ��⵽������
                    curr_state_b <= next_state_b; //bS1
                else
                    counter_e_b <= 21'd0;
            end
        bS1:begin
                if (nege_echo_b)                  // ��⵽�½���
                    curr_state_b <= next_state_b; //bS2
                else
                    begin
                        counter_e_b <= counter_e_b + 1'd1;
                    end
            end
        bS2:begin
                dis_reg_b <= counter_e_b;         // ����������
                counter_e_b <= 0;
                curr_state_b <= next_state_b;     //bS0
            end
        endcase
    end
end

always@(curr_state_b)
begin
    case(curr_state_b)
    bS0:next_state_b <= bS1;
    bS1:next_state_b <= bS2;
    bS2:next_state_b <= bS0;
    endcase
end

//���ҽ�������С��30cm��nr_bΪ��
always@(posedge sys_clk)begin
    if(!echo_en_n)begin
        if(dis_reg_b < 21'd87000 -1'd1) begin
            nr_b<=1'b1;
            nr_b_led<=nr_b;
        end
        else begin
            nr_b<=1'b0;
            nr_b_led<=nr_b;
        end
    end
end

endmodule 