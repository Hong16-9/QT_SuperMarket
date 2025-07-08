**项目标题**

超市收银系统

**描述**

暑期实习项目，包含登陆界面、商品管理、收银界面。使用到sqlite数据库

**功能**

**一、数据库**

  1.属性：无法复制与赋值，构建时初始化且仅一次
  
  2.用户管理
  
    users表内容：
    
                    id（主键） username（唯一非空判断用户类型）password（非空）role（"admin"/"cashier"）
                    
    函数内容：
    
                    creatUser(username,password,role) 
                    
                      为表创建新用户
                    
                    authenticateUser(username,password) 
                    
                       认证得到用户身份
                    
                    数据可视化函数还在设计
  
  3.商品管理
  
    products表内容：
    
                    id（主键） name（非空）barcode（唯一条形码）price（非空>=0）stock（余量非空>=0）category（类型）
    
    函数内容：
    
                    addProduct(const QString& name, const QString& barcode, double price, int stock, const QString& category)
                    
                      新建商品
                    
                    updateProductStock(int productId, int delta)
                    
                      更新商品增加delta个
                    
                    数据可视化函数还在设计
  
  4.会员管理
   
    member表内容：
   
                     id（主键）phone（唯一非空）name（可有可无）discount（0.1到1）
   
    函数内容：
    
                     addMember(const QString& phone, const QString& name, double discount)
     
                       新建会员
       
                     getMemberDiscount(const QString& phone)
       
                       对应折扣，默认折扣为1
 
  5.销售记录
  
    sales表内容：
             
                     id（主键）cashier_id（非空外键到用户id）total（非空>=0）payment（非空>=0）timestamp（销售时间）
 
  6.销售细明
    
     sale_items表内容：
                 
                     sale_id（非空外键到销售记录id）product_id（非空外键到商品id）quantity（非空>=0）price（非空>=0）
  
  7.其他
     
     数据库状态：
       
       isOpen() 返回值bool
       
       lastError() 返回值最后一次操作失败的错误信息

**二、登陆界面**
  
  1.识别登陆账户的类型对应到相应界面，用密码验证
