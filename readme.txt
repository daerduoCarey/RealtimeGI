目录结构：

/------>
	./Data/Data Direct/ : 直接光照采样数据集, 包括原始数据集，训练后的神经网络，以及神经网络的参数

	./Data/Data Direct/ : 间接光照采样数据集, 包括原始数据集，训练后的神经网络，以及神经网络的参数

	./demo/ : 实时场景全局光漫游视频demo x 3

	./Matlab Training Code/ : 神经网络训练源代码

	./Microsoft Stuffs/ : 包括论文以及相关的一些资料

	./Report/ : 生成报告的latex源文件

	./report.pdf : 实验报告

	./Project/ : 渲染项目，包括生成数据集，离线渲染器，GPU shader，实时漫游GUI等。


*重要说明*：由于实时漫游实现在实验室的GPU上，可能对不同的显卡会有一些问题，程序运行需要OpenGL 2.0 Support，以及GPU支持。

实验室机器参数： Intel Core i7-3770 3.4GHz CPU and 8GB memory,AMD Radeon HD 7400 Series GPU. 