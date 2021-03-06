<!-- vscode-markdown-toc -->
* 1. [发现但未解决的问题](#)
	* 1.1. [展开深度](#-1)
		* 1.1.1. [描述](#-1)
		* 1.1.2. [举例](#-1)
	* 1.2. [动态对象](#-1)
		* 1.2.1. [描述](#-1)

<!-- vscode-markdown-toc-config
	numbering=true
	autoSave=true
	/vscode-markdown-toc-config -->
<!-- /vscode-markdown-toc -->
##  1. <a name=''></a>发现但未解决的问题
###  1.1. <a name='-1'></a>展开深度
####  1.1.1. <a name='-1'></a>描述
由于每一个对象可能有若干成员，而每一个成员展开后，又可以有若干成员，某个成员可能是指针类型，从而指向某一父代，而父代再指向子代，就会形成循环展开，导致无穷展开，目前没有想到很好的数据结构来记录这种情况，来避免重复展开的问题，因此暂时设置展开深度的参数，进行展开深度控制，默认展开深度是10。
####  1.1.2. <a name='-1'></a>举例
```cpp
struct Node
{
	std::vector<Node>*list;
	char* data;
};
```
```cpp
Node a1;
Node b1;

a1.list->push_back(b1);
b1.list->push_back(a1);
```
这种情况的a1成员展开后会指向b1，而b1的成员展开后又会指向a1，此种情况会形成无限展开。
当然你可能会说，用一个数据结构记录一下，展开过的不再展开不就好了。
但是实际情况，不会像上面的例子只有两代这么简单，实际的展开可能有十代，而某一代的某一成员可能指向任意某代的某一成员，从而形成错综复杂的无规律网格情况，暂时没有想到很好的办法在不过度损耗效率和内存的情况下，记录而避免这种情况。
###  1.2. <a name='-1'></a>动态对象
####  1.2.1. <a name='-1'></a>描述
由于有些成员在蓝图虚拟机中可能是动态变化的，因此在展开某对象的某一成员时，可能由于该成员实时更新的问题，而无限展开，暂时没有很好的办法记录并避免这种情况。