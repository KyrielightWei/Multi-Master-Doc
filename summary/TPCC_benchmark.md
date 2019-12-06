<!--
 目标：\
tpcc用的表是什么样的，ER图是什么样的\
别人都拿他测什么\
他跑的事务是什么，事务中有哪些sql语句
-->
## 简介
OLTP在线事务处理，TPC-C是对商品销售支付等实际业务系统很好的抽象

> 基准的目的是减少生产应用程序中发现的操作的多样性，同时保留应用程序中发现的操作的多样性。管理生产订单输入系统需要执行大量的功能，但这些功能许多并不是性能分析的主要关注点，在TPC-C中被忽略

TPC-C 模拟应用环境：在线订单处理系统。假设有一个大型商品批发商，拥有多个地理分布的销售区域和相关仓库。每个区域仓库（warehouse)覆盖10个地区（districts），每个每个地区服务3000顾客。所有的仓库都为公司售出的10万件商品储备库存。客户从公司订购新订单或请求现有订单的状态，订单平均由10个订单行组成，所有订单行的百分之一用于区域仓库中没有库存的项目，必须由其他仓库提供。该公司的系统还用于输入来自客户的付款，处理交付订单，并检查库存水平，以确定潜在的供应短缺。

## 表关系

9张表


![TPC-C商业模型ER图](https://upload.wikimedia.org/wikipedia/commons/6/63/Sch%C3%A9ma_Datab%C3%A1ze_metody_TPC-C.png)

**仓库数量w是可变参数，可以通过改变w的值来获得不同测试效果。**

## 表布局
>- **N unique IDs**：至少N个唯一的ID
>- **variable text**, size N ：可变文本，最大长度为N
>- **fixed text**, size N ：固定文本，必须能够保存固定长度为N的任何字符串
>- **date and time**：日期，1900.1.1~2100.12.31
00:00:00~13:59:59
>- **numeric(m,[,n])**：无符号数值，其小数位数总数至少为m，小数点后n位
>- **signed numeric(m,[,n])**：有符号数值，有正负值
>- **null**：空

### WAREHOUSE 仓库表

字段名称 | 字段定义 | 注释
---|---|---
W_ID |  2*W unique IDs |W是仓库数目
W_NAME | variable text, size 10 |仓库名称
W_STREET_1 | variable text, size 20|街道
W_STREET_2 | variable text, size 20|
W_CITY | variable text, size 20 |城市
W_STATE  |fixed text, size 2|州
W_ZIP | fixed text, size 9 |邮政编号
W_TAX | signed numeric(4,4) |销售税
W_YTD | signed numeric(12,2) |本年余额
> Primary Key: W_ID

### DISTRICT 地区表
字段名称 | 字段定义 | 注释
---|---|---
D_ID |20 unique IDs |每个仓库有覆盖10个地区 
D_W_ID |2*W unique IDs |这个地区所属的仓库
D_NAME | variable text, size 10 |地区名称 
D_STREET_1| variable text, size 20|
D_STREET_2  |variable text, size 20|  
D_CITY | variable text, size 20 |城市
D_STATE | fixed text, size 2  |州
D_ZIP | fixed text, size 9  |邮编
D_TAX | signed numeric(4,4) |销售税
D_YTD | signed numeric(12,2) |本年余额
D_NEXT_O_ID | 10,000,000 unique IDs |下一张订单号
> Primary Key: (D_W_ID, D_ID) \
D_W_ID Foreign Key, references W_ID
//外键->仓库表的W_ID

### CUSTOMER 顾客表
字段名称 | 字段定义 | 注释
---|---|---
C_ID | 96,000 unique IDs |每个地区有3000顾客
C_D_ID |20 unique IDs |该顾客所属的地区的编号 
C_W_ID | 2*W unique IDs |对应仓库的编号
C_FIRST | variable text, size 16 |
C_MIDDLE | fixed text, size 2  |
C_LAST | variable text, size 16  |客户姓氏
C_STREET_1 | variable text, size 20| 
C_STREET_2 | variable text, size 20| 
C_CITY | variable text, size 20  |
C_STATE | fixed text, size 2  |
C_ZIP | fixed text, size 9  |
C_PHONE | fixed text, size 16  |手机号
C_SINCE | date and time  |登记日期
C_CREDIT | fixed text, size 2 |信用，"GC"=good, "BC"=bad 
C_CREDIT_LIM | signed numeric(12, 2)  |透支限额
C_DISCOUNT | signed numeric(4, 4)  |折扣
C_BALANCE | signed numeric(12, 2) |欠款余额
C_YTD_PAYMENT | signed numeric(12, 2)  |累计支付金额
C_PAYMENT_CNT | numeric(4)  |累计付款次数
C_DELIVERY_CNT| numeric(4)  |累计发货次数
C_DATA  |variable text, size 500 |备注
>Primary Key: (C_W_ID, C_D_ID, C_ID) //主键->仓库，地区，顾客\
(C_W_ID, C_D_ID) Foreign Key, references (D_W_ID, D_ID) //外键->地区表的D_W_ID, D_ID

### HISTORY 历史记录
字段名称 | 字段定义 | 注释
---|---|---
H_C_ID | 96,000 unique IDs  |
H_C_D_ID | 20 unique IDs  
H_C_W_ID | 2*W unique IDs  
H_D_ID | 20 unique IDs  
H_W_ID | 2*W unique IDs  
H_DATE | date and time  
H_AMOUNT | signed numeric(6, 2)|付款金额
H_DATA | variable text, size 24 |备注，H_DATE是由W_NAME和D_NAME连在一起并用4个空格隔开来构建的
> mary Key: none  //没有主键，因为在基准中不需要唯一标识该表的行 \
(H_C_W_ID, H_C_D_ID, H_C_ID) Foreign Key, references (C_W_ID, C_D_ID, C_ID)  //外键->客户表的C_W_ID, C_D_ID, C_ID \
(H_W_ID, H_D_ID) Foreign Key, references (D_W_ID, D_ID) //外键->地区表的D_W_ID, D_ID

### ORDER 订单表
字段名称 | 字段定义 | 注释
---|---|---
O_ID | 10,000,000 unique IDs  
O_D_ID | 20 unique IDs  
O_W_ID | 2*W unique IDs  
O_C_ID | 96,000 unique IDs  
O_ENTRY_D | date and time |制单时间 
O_CARRIER_ID | 10 unique IDs, or null  |货运代号
O_OL_CNT | numeric(2) |订单行数量，分录数
O_ALL_LOCAL|  numeric(1)  |是否全部本地供货
> Primary Key: (O_W_ID, O_D_ID, O_ID) \
(O_W_ID, O_D_ID, O_C_ID) Foreign Key, references (C_W_ID, C_D_ID, C_ID) //外键->顾客表的C_W_ID, C_D_ID, C_ID

### NEW-ORDER 新订单
字段名称 | 字段定义 | 注释
---|---|---
NO_O_ID | 10,000,000 unique IDs|这个新订单的编号  
NO_D_ID | 20 unique IDs  
NO_W_ID | 2*W unique IDs 
> Primary Key: (NO_W_ID, NO_D_ID, NO_O_ID) \
(NO_W_ID, NO_D_ID, NO_O_ID) Foreign Key, references (O_W_ID, O_D_ID, O_ID) //外键->订单表的O_W_ID, O_D_ID, O_ID

### STOCK 存货
字段名称 | 字段定义 | 注释
---|---|---
S_I_ID | 200,000 unique IDs |每个仓库10万
S_W_ID | 2*W unique IDs  
S_QUANTITY | signed numeric(4)  |库存
S_DIST_01 | fixed text, size 24 |01.02.03...表示地区编号  (OL_D_ID) 
S_DIST_02 | fixed text, size 24  
S_DIST_03 | fixed text, size 24  
S_DIST_04 | fixed text, size 24  
S_DIST_05 | fixed text, size 24  
S_DIST_06 | fixed text, size 24  
S_DIST_07 | fixed text, size 24  
S_DIST_08 | fixed text, size 24  
S_DIST_09 | fixed text, size 24  
S_DIST_10 | fixed text, size 24  
S_YTD | numeric(8) |累计供货数量
S_ORDER_CNT | numeric(4) |累计订单数量
S_REMOTE_CNT | numeric(4) |累计其他仓库供货数量
S_DATA |variable text, size 50 |备注
> Primary Key: (S_W_ID, S_I_ID) \
S_W_ID Foreign Key, references W_ID //外键->仓库表的W_ID\
S_I_ID Foreign Key, references I_ID //外键->商品信息的I_ID

### ORDER-LINE 订单行
字段名称 | 字段定义 | 注释
---|---|---
OL_O_ID | 10,000,000 unique IDs|它对应的订单ID，每个订单平均10个订单行  
OL_D_ID | 20 unique IDs  
OL_W_ID | 2*W unique IDs  
OL_NUMBER | 15 unique IDs |订单行代码
OL_I_ID | 200,000 unique IDs |商品代码
OL_SUPPLY_W_ID | 2*W unique IDs |供货仓库代码 
OL_DELIVERY_D | date and time, or null |发货时间
OL_QUANTITY |numeric(2) |发货数量
OL_AMOUNT | signed numeric(6, 2) |价格
OL_DIST_INFO | fixed text, size 24|S_DIST_xx的值
>  Primary Key: (OL_W_ID, OL_D_ID, OL_O_ID, OL_NUMBER) \
(OL_W_ID, OL_D_ID, OL_O_ID) Foreign Key, references (O_W_ID, O_D_ID, O_ID)//外键->订单表的O_W_ID, O_D_ID, O_ID\
(OL_SUPPLY_W_ID, OL_I_ID) Foreign Key, references (S_W_ID, S_I_ID) //存货表的S_W_ID, S_I_ID

### ITEM 商品信息
字段名称 | 字段定义 | 注释
---|---|---
I_ID | 200,000 unique IDs |10万商品 
I_IM_ID | 200,000 unique IDs |商品图像代码
I_NAME | variable text, size 24 | 
I_PRICE | numeric(5, 2)  |商品价格
I_DATA | variable text, size 50 |品牌信息
> Primary Key: I_ID

## 事务

### 1- The New-Order Transaction 新订单事务 
新订单事务包括通过单个数据库事务输入完整订单，它表示一个中等权重的读写事务，具有较高的执行频率和严格的响应时间要求，**此事务是工作负载的主干**

输入新订单是在单个数据库事务中完成的，步骤如下：

>1. 创建一个订单头，包括：\
2行数据检索选择（++仓库和顾客表++）\
1行数据检索和更新选择（++地区表，下一订单号++）\
2行插入（++订单表、新订单表++）
>2. 订购数量可变的项目（平均ol_cnt=10），包括：\
（1 * ol_cnt）带数据检索的行选择(++商品信息表++）\
（1 * ol_cnt）带数据检索和更新的行选择（++库存表++）\
（1 * ol_cnt）行插入（++订单行表++）

- **输入数据**给SUT（system under test）：\
&emsp;1. D_ID和C_ID（ps：对于任何给定的终端，它的W_ID是确定的）\
&emsp;2. 一组重复字段：OL_I_ID（订单行_商品代码）、OL_SUPPLY_W_ID（订单行_发货仓库代码）和OL_QUANTITY（订单行_发货数量）
- **启动数据库事务**
- **在仓库表中查**匹配的仓库W_ID和税率W_TAX
```
EXEC SQL SELECT c_discount, c_last, c_credit, w_tax  
INTO :c_discount, :c_last, :c_credit, :w_tax 
FROM customer, warehouse 
WHERE w_id = :w_id AND c_w_id = w_id AND c_d_id = :d_id AND c_id = :c_id; 
```
- **在地区表中找**匹配的D_W_ID和D_ID，查它的税率D_TAX和下一可用订单号D_NEXT_O_ID并加一
```
EXEC SQL SELECT d_next_o_id, d_tax
INTO :d_next_o_id, :d_tax 
FROM district 
WHERE d_id = :d_id AND d_w_id = :w_id; 

EXEC SQL UPDATE district 
SET d_next_o_id = :d_next_o_id + 1
WHERE d_id = :d_id AND d_w_id = :w_id;
```
- **在顾客表中查找**匹配的C_W_ID、C_D_ID和C_ID，并检索客户的折扣率C_DISCOUNT、客户的姓氏C_LAST和客户的信用状态C_CREDIT
- 将这个新行**插入到新订单表和订单表**中，创建订单。其中O+CARRIER_ID货运ID为空，如果订单里全部都是由主仓库供货，则O_ALL_LOCAL设为1，否则为0
```
o_id=d_next_o_id;

EXEC SQL INSERT INTO ORDERS (o_id, o_d_id, o_w_id, o_c_id,o_entry_d, o_ol_cnt, o_all_local)
VALUES (:o_id, :d_id, :w_id, :c_id, :datetime, :o_ol_cnt, :o_all_local);//货运单号O_CARRIER_ID为空

EXEC SQL INSERT INTO NEW_ORDER (no_o_id,no_d_id, no_w_id) 
VALUES (:o_id, :d_id, :w_id);//ol_cnt是终端模拟器生成的 
 ```
- 计算O_OL_CNT订单行数目
- 遍历订单的**每个订单行**：\
&emsp;1. 在商品表中查它的商品信息I_ID，检索项的价格I_PRICE、项的名称I_NAME和I_DATA。**如果有没找到的ID，就会发出信号导致事务回滚**\
&emsp;2. **库存表中查**匹配S_I_ID和S_W_ID的行。如果S_QUANTITY-OL_QUANTITY>10，则S_QUANTITY=S_QUANTITY-OL_QUANTITY，否则S_QUANTITY=S_QUANTITY-OL_QUANTITY+91，累计供货数量S_YTD=S_YTD+OL_QUANTITY，S_ORDER_CNT+1，从其他仓库调货就S_REMOTE_CNT+1\
&emsp;3. 订单中的金额OL_amount=OL_QUANTITY*I_PRICE\
&emsp;4.检查I_DATA和S_DATA中的字符串。如果它们都包含字符串“ORIGINAL”，则该项的brand generic字段设置为“B”，否则，brand generic字段设置为“G”。\
&emsp;5.**在订单行表中插入新行**以反映订单上的项目,发货时间OL_DELIVERY_D设空，设分录代码OL_NUMBER，OL_DIST_INFO设为S_DIST_xx的内容，其中xx表示地区编号 (OL_D_ID) 
```
for (ol_number=1;ol_number<=o_ol_cnt;ol_number++)
{
    ol_supply_w_id=atol(supware[ol_number-1]);
    if (ol_supply_w_id != w_id) o_all_local=0;
    ol_i_id=atol(itemid[ol_number-1]);//商品信息id
    ol_quantity=atol(qty[ol_number-1]);//数量
    
    EXEC SQL WHENEVER NOT FOUND GOTO invaliditem;//未找到，事务回滚
    
    EXEC SQL SELECT i_price, i_name , i_data
    INTO :i_price, :i_name, :i_data
    FROM item
    WHERE i_id = :ol_i_id;
    price[ol_number-1] = i_price;
    strncpy(iname[ol_number-1],i_name,24);
    //检索商品信息
    
    EXEC SQL WHENEVER NOT FOUND GOTO sqlerr;//出错
    
    EXEC SQL SELECT s_quantity,s_data,s_dist_01,s_dist_02, s_dist_03, s_dist_04, s_dist_05, s_dist_06,s_dist_07,s_dist_08, s_dist_09, s_dist_10
    INTO :s_quantity, :s_data,:s_dist_01,:s_dist_02, :s_dist_03,:s_dist_04,:s_dist_05,:s_dist_06,:s_dist_07,:s_dist_08,:s_dist_09,:s_dist_10
    FROM stock
    WHERE s_i_id = :ol_i_id AND s_w_id = :ol_supply_w_id;
    
    pick_dist_info(ol_dist_info, ol_w_id); // pick correct s_dist_xx
    stock[ol_number-1] = s_quantity;
    if ( (strstr(i_data,"original") != NULL) &&(strstr(s_data,"original") != NULL) )
        bg[ol_number-1] = 'B';
    else
        bg[ol_number-1] = 'G';
        
    if (s_quantity > ol_quantity)   //s_quantity - ol_quantity > 10
        s_quantity = s_quantity - ol_quantity;
    else
        s_quantity = s_quantity - ol_quantity + 91;
    EXEC SQL UPDATE stock 
    SET s_quantity = :s_quantity
    WHERE s_i_id = :ol_i_id AND s_w_id = :ol_supply_w_id;
    //更新库存信息
    
    ol_amount = ol_quantity * i_price * (1+w_tax+d_tax) * (1-c_discount);//计算订单行金额
    amt[ol_number-1]=ol_amount;
    total += ol_amount;//完整订单的金额
    
    EXEC SQL INSERT INTO order_line (ol_o_id, ol_d_id, ol_w_id, ol_number, ol_i_id, ol_supply_w_id, ol_quantity, ol_amount, ol_dist_info)
    VALUES (:o_id, :d_id, :w_id, :ol_number,:ol_i_id,:ol_supply_w_id,:ol_quantity,:ol_amount, :ol_dist_info);
    //在订单行中插入新行反映订单上的信息
    
} /*End Order Lines*/

invaliditem:
EXEC SQL ROLLBACK WORK;
printf("Item number is not valid");
return(0);

sqlerr:
error();
```
- 完整订单的总金额计算为：sum(OL_AMOUNT) * (1 - C_DISCOUNT) * (1 + W_TAX + D_TAX) 
- **事务提交**
```
EXEC SQL COMMIT WORK;
return(0);
```
- **输出数据**：\
&emsp;1. W_ID、D_ID、C_ID、O_ID、O_OL_CNT、C_LAST、C_CREDIT、C_DISCOUNT、W_TAX、D_TAX、O_ENTRY_D、total_amount，以及除“项目编号无效”之外的可选执行状态消息\
&emsp;2. 重复字段：OL_SUPPLY_W_ID, OL_I_ID, I_NAME, OL_QUANTITY, S_QUANTITY, 
brand_generic, I_PRICE, and OL_AMOUNT. \
&emsp;3. 对于由于未使用的项目号而回滚的事务（占所有新订单事务的1%），模拟终端必须在输入/输出屏幕的相应字段中显示以下字段：W_ID、D_ID、C_ID、C_LAST、C_CREDIT、O_ID，以及执行状态消息“项目号无效”。

### 2- The Payment Transaction 支付事务
更新客户余额，在地区和仓库销售统计中反映付款情况。它代表一个轻量级的读写事务，具有高执行频率和响应时间要求。此事务还包括对顾客表非主键的访问

> 情况1：根据客户编号选择\
&emsp;3行选择，数据检索更新（++客户表++）\
&emsp;一行新增（++历史记录表++）\
情况2：根据客户姓氏选择\
&emsp;平均2行数据检索（++客户表++，在姓氏中找）\
&emsp;3行数据检索和更新（++客户表++）\
&emsp;1行插入（++历史记录表++）


- **输入数据** 传给SUT：D_ID, C_ID or C_LAST, C_D_ID, C_W_ID, 和付款金额 H_AMOUNT. 
- **启动事务**
- **在仓库表中查**匹配的W_ID，检索其W_NAME, W_STREET_1, 
W_STREET_2, W_CITY, W_STATE, 和 W_ZIP，更新仓库本年余额W_YTD = W_YTD + H_ AMOUNT. 
```
EXEC SQL UPDATE warehouse 
SET w_ytd = w_ytd + :h_amount
WHERE w_id=:w_id;//更新仓库本年余额 += 支付金额

EXEC SQL SELECT w_street_1, w_street_2, w_city, w_state, w_zip, w_name
INTO :w_street_1, :w_street_2, :w_city, :w_state, :w_zip, :w_name
FROM warehouse
WHERE w_id=:w_id;
```
- **在地区表中查**匹配的D_W_ID and D_ID ，检索D_NAME, D_STREET_1, D_STREET_2, D_CITY, D_STATE, 以及 D_ZIP ，更新D_YTD=D_YTD+H_AMOUNT
```
EXEC SQL UPDATE district 
SET d_ytd = d_ytd + :h_amount
WHERE d_w_id=:w_id AND d_id=:d_id;

EXEC SQL SELECT d_street_1, d_street_2, d_city, d_state, d_zip, d_name
INTO :d_street_1, :d_street_2, :d_city, :d_state, :d_zip, :d_name
FROM district
WHERE d_w_id=:w_id AND d_id=:d_id;
```
- **在顾客表中查找**\
情况1：根据客户编号查找客户,匹配C_W_ID, C_D_ID and C_ID，检索C_FIRST, C_MIDDLE, C_LAST, C_STREET_1, C_STREET_2, C_CITY, C_STATE, C_ZIP, C_PHONE, C_SINCE, C_CREDIT, C_CREDIT_LIM, C_DISCOUNT, 和 C_BALANCE ，顾客累计支付金额C_YTD_PAYMENT=C_YTD_PAYMENT + H_AMOUNT，累计付款次数C_PAYMENT_CNT +1
```
EXEC SQL SELECT c_first, c_middle, c_last,c_street_1, c_street_2, c_city, c_state, c_zip,c_phone, c_credit, c_credit_lim,c_discount, c_balance, c_since
INTO :c_first, :c_middle, :c_last,:c_street_1,:c_street_2, :c_city, :c_state, :c_zip,:c_phone, :c_credit,:c_credit_lim,:c_discount, :c_balance,:c_since
FROM customer
WHERE c_w_id=:c_w_id AND c_d_id=:c_d_id AND c_id=:c_id;
```
情况2：根据姓氏查找客户客户表中所有匹配C_W_ID、C_D_ID和C_LAST的行按C_FIRST的升序排列，令n为所选行数，从CUSTOMER表中已排序的选定行集合中**位于位置（n/2向上舍入到下一个整数）的行**中检索C_BALANCE=C_BALANCE -H_AMOUNT，C_YTD_PAYMENT =C_YTD_PAYMENT+H_AMOUNT。C_PAYMENT_CNT+1。
```
if (byname)
{
    EXEC SQL SELECT count(c_id) 
    INTO :namecnt
    FROM customer
    WHERE c_last=:c_last AND c_d_id=:c_d_id AND c_w_id=:c_w_id;
    
    EXEC SQL DECLARE c_byname CURSOR FOR
    SELECT c_first, c_middle, c_id, c_street_1, c_street_2, c_city, c_state, c_zip, c_phone, c_credit, c_credit_lim,c_discount, c_balance, c_since
    FROM customer
    WHERE c_w_id=:c_w_id AND c_d_id=:c_d_id AND c_last=:c_last
    ORDER BY c_first;//按first升序排列
    
    EXEC SQL OPEN c_byname;
    if (namecnt%2) namecnt++; // Locate midpoint customer;
    for (n=0; n<namecnt/2; n++)
    {
        EXEC SQL FETCH c_byname
        INTO :c_first, :c_middle, :c_id,:c_street_1, :c_street_2, :c_city, :c_state, :c_zip,:c_phone, :c_credit, :c_credit_lim,:c_discount, :c_balance, :c_since;
    }
    EXEC SQL CLOSE c_byname;
}
```

```
c_balance += h_amount;

/*感觉这里不太对，应该是：
c_balance -= h_amount;
c_ytd_payment += h_amount;
c_payment_cnt +1;*/
```
- 如果C_CREDIT的值等于“BC”，还要改变其值
```
c_credit[2]='\0';
if (strstr(c_credit, "BC") )
{
    EXEC SQL SELECT c_data INTO :c_data
    FROM customer
    WHERE c_w_id=:c_w_id AND c_d_id=:c_d_id AND c_id=:c_id;
    
    sprintf(c_new_data,"| %4d %2d %4d %2d %4d $%7.2f %12c %24c",c_id,c_d_id,c_w_id,d_id,w_id,h_amount,h_date, h_data);
    strncat(c_new_data,c_data,500-strlen(c_new_data));
    EXEC SQL UPDATE customer
    SET c_balance = :c_balance, c_data = :c_new_data
    WHERE c_w_id = :c_w_id AND c_d_id = :c_d_id AND c_id = :c_id;
}
else
{
    EXEC SQL UPDATE customer 
    SET c_balance = :c_balance
    WHERE c_w_id = :c_w_id AND c_d_id = :c_d_id AND c_id = :c_id;
}
```
- H_DATE是由W_NAME和D_NAME连在一起并用4个空格隔开来构建的
```
strncpy(h_data,w_name,10);
h_data[10]='\0';
strncat(h_data,d_name,10);
h_data[20]=' ';
h_data[21]=' ';
h_data[22]=' ';
h_data[23]=' ';
```
- **插入到HISTORY表**
```
EXEC SQL INSERT INTO history (h_c_d_id, h_c_w_id,h_c_id, h_d_id,h_w_id, h_date, h_amount, h_data)
VALUES (:c_d_id, :c_w_id, :c_id, :d_id,:w_id, :datetime, :h_amount, :h_data);
```
- **提交事务**
- **输出数据**W_ID, 
D_ID, C_ID, C_D_ID, C_W_ID, W_STREET_1, W_STREET_2, W_CITY, W_STATE, W_ZIP, D_STREET_1, 
D_STREET_2, D_CITY, D_STATE, D_ZIP, C_FIRST, C_MIDDLE, C_LAST, C_STREET_1, C_STREET_2, C_CITY, 
C_STATE, C_ZIP, C_PHONE, C_SINCE, C_CREDIT, C_CREDIT_LIM, C_DISCOUNT, C_BALANCE, the first 200 
characters of C_DATA (only if C_CREDIT = "BC"), H_AMOUNT, and H_DATE. 

### 3- The Order-Status Transaction 订单状态查询事务
查询用户上一个订单的状况，中等权重只读事务，具有较低的执行频率和响应时间要求
>1. 查找客户以及他的上一个订单，包括：\
情况1：以客户编号查询，2行检索\
&emsp;情况2：以客户姓氏查询，平均4行检索
>2. 检查订单上每个项目的状态（平均每个订单10个项目），包括：\
（1*每个订单的项目数）行选择和数据检索
- **输入数据** D_ID ，C_ID 或C_LAST. 
- **事务开始**
- **在客户表中查找**客户信息\
情况1，以客户编号查询

```
EXEC SQL SELECT c_balance, c_first, c_middle, c_last
INTO :c_balance, :c_first, :c_middle, :c_last
FROM customer
WHERE c_id=:c_id AND c_d_id=:d_id AND c_w_id=:w_id;
```
情况2，以客户姓氏查询
```
if (byname)
{
    EXEC SQL SELECT count(c_id) 
    INTO :namecnt
    FROM customer
    WHERE c_last=:c_last AND c_d_id=:d_id AND c_w_id=:w_id;
    
    EXEC SQL DECLARE c_name CURSOR FOR
    SELECT c_balance, c_first, c_middle, c_id
    FROM customer
    WHERE c_last=:c_last AND c_d_id=:d_id AND c_w_id=:w_id
    ORDER BY c_first;
    
    EXEC SQL OPEN c_name;
    if (namecnt%2) namecnt++; // Locate midpoint customer
    for (n=0; n<namecnt/2; n++)
    {
        EXEC SQL FETCH c_name
        INTO :c_balance, :c_first, :c_middle, :c_id;
    }
    EXEC SQL CLOSE c_name;
}
```
- **在订单表中查找**，匹配O_W_ID ， O_D_ID , O_C_ID ，找出现有的最大的O_ID，这代表是客户最近下的订单，检索O_ID, O_ENTRY_D, 和 O_CARRIER_ID 
```
EXEC SQL SELECT o_id, o_carrier_id, o_entry_d
INTO :o_id, :o_carrier_id, :entdate
FROM orders
ORDER BY o_id DESC;
```
- 根据订单的信息**在订单行表中检索**OL_I_ID, OL_SUPPLY_W_ID, 
OL_QUANTITY, OL_AMOUNT, and OL_DELIVERY_D
```
EXEC SQL DECLARE c_line CURSOR FOR
SELECT ol_i_id, ol_supply_w_id, ol_quantity,ol_amount, ol_delivery_d
FROM order_line
WHERE ol_o_id=:o_id AND ol_d_id=:d_id AND ol_w_id=:w_id;

EXEC SQL OPEN c_line;
EXEC SQL WHENEVER NOT FOUND CONTINUE;
i=0;
while (sql_notfound(FALSE))
{
    i++;
    EXEC SQL FETCH c_line
    INTO :ol_i_id[i], :ol_supply_w_id[i],:ol_quantity[i],:ol_amount[i], :ol_delivery_d[i];
}
EXEC SQL CLOSE c_line;
```
- **事务提交**
- **输出数据**\
&emsp;W_ID, D_ID, C_ID, C_FIRST, C_MIDDLE, C_LAST, C_BALANCE, O_ID,
O_ENTRY_D, 和 O_CARRIER_ID; \
&emsp;多个订单行数据OL_SUPPLY_W_ID, OL_I_ID, OL_QUANTITY, OL_AMOUNT, 和OL_DELIVERY_D. 

###  4- The Delivery Transaction 发货事务
处理一批新订单（未交货的），业务由**1-10个数据库事务组成**（可以在一个事务中完成，也可以分解到多达10个事务中），执行频率低，响应时间宽松。

**Deferred Execution延迟执行**，通过排队机制，终端的相应指示事务完成，结果记录到结果文件
>1. 处理订单，包含：\
1行数据检索\
（1+每个订单所含的货物数量）行数据检索更新
>2. 更新客户余额，包含：\
1行数据检索更新
>3. 从新订单表中移除该订单，包含：\
1行删除操作

- **输入数据**： O_CARRIER_ID
- **事务开始**
- **在NEW-ORDER表中查找**匹配NO_W_ID和NO_D_ID的行，且NO_O_ID最小（订单号最小代表该订单最老）。如果没有找到匹配的行则跳过，在给定区域没有未完成订单的情况下，必须通过跳过仅该区域的订单交付并从选定仓库的所有剩余区域恢复订单交付来处理。
```
EXEC SQL DECLARE c_no CURSOR FOR
SELECT no_o_id
FROM new_order
WHERE no_d_id = :d_id AND no_w_id = :w_id
ORDER BY no_o_id ASC;
```
- **在新订单表中删除**该行
```
EXEC SQL OPEN c_no;
EXEC SQL WHENEVER NOT FOUND continue;

EXEC SQL FETCH c_no 
INTO :no_o_id;

EXEC SQL DELETE FROM new_order WHERE CURRENT OF c_no;//当前游标

EXEC SQL CLOSE c_no;
```
- **在订单表中查找**匹配O_W_ID，O_D_ID和O_ID的行，检索O_C_ID，并且更新O_CARRIER_ID
```
EXEC SQL SELECT o_c_id INTO :c_id FROM orders
WHERE o_id = :no_o_id AND o_d_id = :d_id AND o_w_id = :w_id;

EXEC SQL UPDATE orders 
SET o_carrier_id = :o_carrier_id
WHERE o_id = :no_o_id AND o_d_id = :d_id AND
o_w_id = :w_id;
```
- **在订单行表中查找**匹配OL_W_ID，OL_D_ID和OL_O_ID的行，所有的OL_DELIVERY_D更新为系统当前时间，检索OL_AMOUNT
```
EXEC SQL UPDATE order_line 
SET ol_delivery_d = :datetime
WHERE ol_o_id = :no_o_id AND ol_d_id = :d_id AND ol_w_id = :w_id;

EXEC SQL SELECT SUM(ol_amount)
INTO :ol_total
FROM order_line
WHERE ol_o_id = :no_o_id AND ol_d_id = :d_id AND ol_w_id = :w_id;
```
- **在顾客表中查找**匹配C_W_ID，C_D_ID和C_ID的行，C_BALANCE=C_BALANCE+sum（所有订单行的OL_AMOUNT），C_DELIVERY+1
```
EXEC SQL UPDATE customer 
SET c_balance = c_balance + :ol_total
WHERE c_id = :c_id AND c_d_id = :d_id AND c_w_id = :w_id;
```
- 如果在这个事务中不继续提交订单，就**提交事务**
- 将有关信息记录在**结果文件**中

### 5- The Stock-Level Transaction 库存水平查询事务
检查最近20个订单上项目的库存水平，确定库存水平低于阈值的最近售出项目的数量。执行频率低，相应时间要求宽松，一致性要求宽松的大型只读事务
> 1.检查下一个可用的订单号，包含：\
一行数据检索\
2.检查区域最近20个订单上的所有项目，包括：\
（20*每个订单的项目数）行数据检索\
3.对于所选的每个货物，如果主仓库的可用库存水平低于阈值，则检查包括：\
最多（20*每个订单的项目数）行数据检索

- **输入数据**：threshold阈值
- **事务开始**
- **在区域表中查找**匹配D_W_ID和D_ID的行，检索其D_NEXT_O_ID
```
EXEC SQL SELECT d_next_o_id 
INTO :o_id
FROM district
WHERE d_w_id=:w_id AND d_id=:d_id;
```
- **在订单行表中查找**匹配OL_W_ID，OL_D_ID和OL_O_ID（D_NEXT_O_ID-20 =< OL_O_ID < D_NEXT_O_ID\
这是本区域最近20个订单包含的货品
- **在库存表中查找**匹配S_I_ID、S_W_ID的行，且S_QUANTITY < 阈值
```
EXEC SQL SELECT COUNT(DISTINCT (s_i_id)) 
INTO :stock_count
FROM order_line, stock
WHERE ol_w_id=:w_id AND ol_d_id=:d_id AND ol_o_id<:o_id AND ol_o_id>=:o_id-20 AND s_w_id=:w_id AND s_i_id=ol_i_id AND s_quantity < :threshold;
```
- **提交事务**
- **输出数据**W_ID, 
D_ID, threshold, and low_stock. 


## 性能指标和响应时间

交易类型 | 最低混合百分比|最低键入事件|90%事务响应时间要求|最小平均思考时间分布
---|---|---|---|---|---
New-Order | n/a|18s|5s|12s
Payment | 43.0|3s|5s|12s
Order-Status | 4.0|2s|5s|10s
Delivery | 4.0|2s|5s|5s
Stock-Level | 4.0|2s|20s|5s
响应时间用于终端响应（确认事务已排队），而不是用于执行事务本身。至少90%的事务必须在排队后80秒内完成
> RT(Response Times)=T2-T1，其中T2=终端接收到输出数据的最后一个字符后的时间戳，T1=用户输入数据的最后一个字符前的时间戳\
Menu RT（菜单响应时间）=接收到屏幕最后一个字符之后的时间戳 - 输入菜单选择的最后一个字符之前的时间戳\
Transaction RT（事务响应时间）=RTE（remote terminal emulator 远程终端仿真器）接受的所需输出数据的最后一个字符之后的时间戳 - 从RTE发出发送数据的最后一个字符之前的时间戳\
思考时间：独立于负指数分布 p72


## 主要度量
- **每分钟处理的订单数量**=最大合格吞吐量（MQTH）
单位：tpmC\
是用户在执行payment、order-status、delivery和stock-level这四种交易的同时，每分钟可以处理的new-order交易的数量。\
由图可以看出，新交易最高占比45%（100-43-4-4-4）
- **性价比**：tpc-c三年总定价/MQTH，单位$/tpmC
- 可用日期
- 当使用可选标准时，报告附加指标

### 所需报告
1. 对于每个交易，必须报道开始和完成的所有交易的响应时间的频率分布。x轴-RT值（90%是N，最大4N），y轴-频率分布
2. 最大吞吐量和响应时间的关系
3. 思考时间的频率分布
4. 吞吐量和运行时间的关系
