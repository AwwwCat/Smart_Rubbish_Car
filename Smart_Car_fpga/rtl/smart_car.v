module smart_car(
    input               sys_clk  ,              //ϵͳʱ��
    input               sys_rst_n,              //ϵͳ��λ���͵�ƽ��Ч
    input               echo_a,                 //�����ź�echo_a
    input               echo_b,                 //�����ź�echo_b
    input               echo_en_n,              //�������ʹ�ܣ��͵�ƽ��Ч
    input               a,b,                    //С�������ź�
    input               p1,p2,p3,               //��������ź�
    input               en_ab,                  //С���˶�ʹ��
    output  reg         pwm_ab,                 //С���˶�pwm
    output  reg         pwm1,pwm2,pwm3,pwm4,    //��������ź�
    output  reg         l1,l2,l3,l4,            //С�������ź�
    output  reg         trig,                   //�����ź�echo_a,echo_b����
    output  reg         nr_a,nr_a_led,
    output  reg         nr_b,nr_b_led           //nr_a,nr_b���ҽ���С��10cmΪ��
    );

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
always @(sys_clk, p1, p2, p3)begin
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
                    pwm4<=1'b0;        //����
        end
    else if((!p2) & p3)
        begin
                if(counter_p<20'd45000)
                    pwm4<=1'b1;
                else
                    pwm4<=1'b0;        //����
        end
    else
        begin
                if(counter_p<20'd125000)
                    pwm4<=1'b1;
                else
                    pwm4<=1'b0;        //����
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
            if(counter_p<20'd500000-1'd1)
                pwm_ab<=1'b1;
            else
                pwm_ab<=1'b0;
            end
        else
            pwm_ab<=1'b0;
    end
/************************************trig ģ��************************************/
reg [20:0] counter_t;                 //trig�����ڼ���                                                                                                                                                                                 
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
assign pose_echo_a =(~echo_a2)&&echo_a1;//echo_a �����½���
assign nege_echo_a = echo_a2&&(~echo_a1);
parameter aS0 = 2'b00, aS1 = 2'b01, aS2 = 2'b10; // echo_a ״̬���� aS0:����, aS1:��ʼ������, aS2:����������
reg[1:0] curr_state_a, next_state_a;
reg [20:0]    counter_e_a;               //echo_a�ĸߵ�ƽ����
reg [20:0]    dis_reg_a;                 //����echo_a�ߵ�ƽ����ֵ

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
        echo_a1 <= echo_a;          // ��ǰ
        echo_a2 <= echo_a1;     // ��һ��
        case(curr_state_a)
        aS0:begin
                if (pose_echo_a) // ��⵽������
                    curr_state_a <= next_state_a; //aS1
                else
                    counter_e_a <= 21'd0;
            end
        aS1:begin
                if (nege_echo_a) // ��⵽�½���
                    curr_state_a <= next_state_a; //aS2
                else
                    begin
                        counter_e_a <= counter_e_a + 1'd1;
                    end
            end
        aS2:begin
                dis_reg_a <= counter_e_a; // ����������
                counter_e_a <= 0;
                curr_state_a <= next_state_a; //S0
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
assign pose_echo_b =(~echo_b2)&&echo_b1;//echo_b �����½���
assign nege_echo_b = echo_b2&&(~echo_b1);
parameter bS0 = 2'b00, bS1 = 2'b01, bS2 = 2'b10; // echo_b ״̬���� bS0:����, bS1:��ʼ������, bS2:����������
reg[1:0] curr_state_b, next_state_b;
reg [20:0]    counter_e_b;               //echo_b�ĸߵ�ƽ����
reg [20:0]    dis_reg_b;                 //����echo_b�ߵ�ƽ����ֵ

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
        echo_b1 <= echo_b;          // ��ǰ
        echo_b2 <= echo_b1;     // ��һ��
        case(curr_state_b)
        bS0:begin
                if (pose_echo_b) // ��⵽������
                    curr_state_b <= next_state_b; //bS1
                else
                    counter_e_b <= 21'd0;
            end
        bS1:begin
                if (nege_echo_b) // ��⵽�½���
                    curr_state_b <= next_state_b; //bS2
                else
                    begin
                        counter_e_b <= counter_e_b + 1'd1;
                    end
            end
        bS2:begin
                dis_reg_b <= counter_e_b; // ����������
                counter_e_b <= 0;
                curr_state_b <= next_state_b; //S0
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