# DNS大作业（北邮）
## 支持中文字符的查询，采用迭代查询
测试说明：<br>
	1.	make命令进行编译<br><br>
	2.	运行dns时，argv[1] = IP地址，argv[>1] = 数据文档名 （注意：需要一个IP地址为127.5.2.1，作为root节点）<br> <br>
	![image](https://github.com/zxhero/DNS/blob/master/example.PNG)<br> <br> <br>
	3.	运行local_dns时，不需要参数<br> <br>
	4.	运行client时，不需要参数<br> <br>
	5.	数据文档编写方式：<br> <br>
		域名 类型 集合 保存时间 数据长度 数据<br> <br>
		例如：	主页.北邮.教育.中国 1 1 20 12 192.168.1.25<br> <br>
	6.	关于集合和类型的定义详见dns_protocal.h<br> <br>
	![image](https://github.com/zxhero/DNS/blob/master/definition.PNG)<br> <br> <br>
	7.	cache的刷新时间为20sec<br> <br>
	8.	关于cache中的内容详见cache.txt
	9.	client的输入格式：（当输入 0 0 时，程序停止运行）<br> <br>
		域名 类型<br> <br>
		例如：	主页.北邮.教育.中国 A<br> <br>
