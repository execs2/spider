##某个社交app爬虫

主要用libcurl实现的一个爬虫，写这个主要因为好玩，绝不是程序员的闷骚。。哈哈哈

抓取某个社交app用户（只抓取女性用户。。。）的头像图片、基本信息、相册、动态（包括动态文本和图片）。

多线程，线程之间无锁（只在获取id的时候使用了__sync_fetch_and_add），抓取速度快，基本满带宽下载


##INSTALL:
```Bash
./makefile
```

##Usage:
```Bash
spider -t thread_count -p save path -i start_id
```

thread_count：线程数，可以看情况多开点
save：保存的路径
start_id：抓取的用户起始id，不会重复抓取已经抓取的用户

注：我已经很久没更新，这个目标app的有些api可能已经更改，所以可能会失效。但是代码可以参考。


##我的邮箱：
  
646452100@qq.com
