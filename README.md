

# TODO
- [ ] log_util 完善
- [ ] lcs, bm25 等相关性计算方法
- [ ] 多队列召回
- [ ] 机器学习在搜索中的应用介绍
- [ ] 深度学习在搜索中的应用介绍


# What's this?
* 一个简单搜索引擎。优化排序算法，通过NDCG评估排序效果。
* 主要主要关注搜索排序，不涉及其他部分，如爬虫、建库、前端展现等
* 主要实现检索功能，架构设计较简单，不涉及分布式、多线程等
* 为简单起见，除`jiebacpp`外，基本不依赖其他第三方库，这么做的目的是聚焦于搜索引擎本身，够用即可，不必被其他复杂的功能束缚住。实际应用中，需要使用成熟的组件，如`boost`, `glog`, `gflags`, `gtest`, `protobuf`, `RapidJson`等
* 整理了常见的停用词词表, 包括中文、英文、标点符号, 在`dict/stopword.chinese`, `dict/stopword.english`, `dict/stopword.punctuation` 目录下, 其中中文为utf-8编码。

# 开发环境
* CentOS release 6.3
* Linux version 3.10
* gcc version 7.5 (GCC)

# Getting Started
```
make
./bin/main
# 输入query即可搜索
```

## demo
![](https://github.com/MarinYoung4596/tiny_search_engine/blob/main/pic/demo.png)

# 迭代 & 评估
## 训练集

1. 从`http://top.baidu.com`、`http://top.sogou.com/`、`https://www.zhihu.com/hot` 等站点抓取热门query
2. 用上述query去百度抓取top10搜索结果（考虑到百度搜索结果中有很多阿拉丁特型展现，因此实际抓取top20结果）
3. 解析url、title等
4. 按照`相关性`、`时效性`、`权威性`、`内容质量`等维度对url进行打分
5. 得到标注集合：`query  \t  url  \t  title  \t  label`


## 评估效果(NDCG@3)

| 版本  | 测试集指标   | note               |
| ---- | ----------  | ------------------ |
| V0.1 | 72.08; 492 | cqr * ctr          |
| v0.3 | 73.58; 492 | cqr * ctr; stopword |
| v0.6 | 73.89; 491 | vsm 增加停用词; |
| v0.7 | 73.99; 494 | term频次交集取最小   |


## badcase分析
### 1. q=电脑美容哪个公司好
 * ranking结果中top3为：
    * a) 汽车美容和电脑维修哪个好
    * b) 雅培和惠氏哪个好啊
    * c) 惠氏和雅培哪个比较好
 * doc集合中比较好的结果有：
    * d) 贵阳市哪里有给笔记本电脑美容的店 地址电话最好有
    * e) 供应微电脑美容bio美容仪// 效果最好 价格最低 海口美
 * d) 排序靠后, 原因是切词粒度：
    * query切成`电脑 | 美容 | 哪个 | 公司 | 好`
    * title切成`贵阳市 | 哪里 | 有 | 给 | 笔记本电脑 | 美容 | 的 | 店 |   | 地址 | 电话 | 最好 | 有`
    * 因此实际上，命中的term只有"美容"一个，显然不符合预期；
* 解决方法：
    1. 引入term前后继关系偏移距离解决。e.g. a) 与 e) 同样命中2个term, 很明显前者不相关. 观察发现，前者term间的距离为-3（以term首个字符的偏移量计算）, 后者term间偏移距离为2;
    2. 引入term紧密度解决。例如：中国/银行，两个term紧密相关，不能切开，中国/工商/银行 这样的url并非紧密命中，相关性可以打压；

### 2. q=金华新楼盘售价
 * ranking结果中top3为：
   * a) 2011年3月7日贵阳各区县楼盘最新售价一览
   * b) 浙江金华新楼盘均价屡过万 "价格凹地"被填平
   * c) 金华新楼盘均价屡屡过万 价格凹地被慢慢填平
 * 明显，a) 不相关，排在top1原因是：1）term重要性计算不准，"金华" 应该是主体, 这是因为计算idf的数据集较小导致的，加大数据集可以解决；2）b) c) "价格" 跟 "售价" 意思相近，却没有命中, 导致相关性算不上去。
 * 解决方法：引入同义改写term辅助相关性计算。

### 3. q=中关村附近有什么美食
  * 候选doc
    * a) 中关村-餐馆
    * b) 李家村附近有什么美食
  * 显然a)更相关，但是b)命中的term更多，即使每个term的term重要性均比较低，但累加起来的相关性很有可能比a)更高，导致二者逆序。
  * 解决方法：query分析出不可省term，对于未命中不可省term的url，召回阶段直接抛弃。

# 讨论
搜索引擎均为“金字塔”结构（或者“漏斗”结构）：完整的搜索过程在召回阶段后，通常会进行多次排序、过滤、截断等操作。这是由于性能、架构设计等多种因素决定的。越往后，单条结果计算的越精细，耗时越长，计算的结果数越少。

下面将简要介绍搜索中的主要工作。

## 1. query分析
  query分析是搜索的基础。通常query分析需要做如下工作：
* **纠错**：
  * 如何进行纠错？
* **切词**：尤其对于中文搜索而言，每一个词称之为“term”。召回时以term为粒度去倒排索引里召回term拉链。切词的准确性尤为重要。通常情况下，term切词粒度应该越细越好，否则会出现 "电脑" 无法召回 "笔记本电脑" 这样的问题；当然，切词过细也会带来散乱命中的问题。
  * 如何处理新词发现问题？
  * 如何处理二义性问题？e.g. “南京市/长江/大桥” or "南京/市长/江大桥"
  * ...
* **term重要性**：
  * //TODO
* **term改写**：
  * //TODO
* **紧密度**：描述两个term之间的紧密程度。例如 Q=“中国/银行”, 要求两个term紧密命中，否则会召回诸如 “中国工商银行” 之类的转义结果；
* **query embedding**：
  * //TODO
* **query需求分析**
  * 如何识别图片/视频/资讯/小说/下载/寻址等需求？
  * 需求多样query下，如何确定主次需求？
    * 有的query需求是动态变化的，如“范冰冰”，正常可能百科需求、寻址（微博）需求；当主演的电影（如《我不是潘金莲》）上映时，用户想要的是相关的视频片段，这时是视频需求；当偷税漏税事件爆发后，这时主需求是相关新闻报导。
  * 如何判断一个query是否有时效性需求？
  * ...


## 2. doc分析
* TODO

## 3. 召回
 * TODO

## 4. 过滤
 * TODO

## 5. 去重
顾名思义，去除重复结果，保证最终展现结果的多样性。这里的重复指：
* **完全相同**：标题、内容、url 完全相同，这种是绝对需要杜绝的，一般不会出现这样的问题。url不同，其他相同的情况下，需要保护原创内容；
* **内容相同**：标题不同，内容完全相同。正文元素包括 文本、图片、视频等。通过计算内容签名即可去重；
* **标题相同**：标题完全相同，内容不同。从产品维度上考虑，标题相同会降低用户点击的欲望，即使内容不同，一般会展示内容更优质的那条url，而非同时展现；
* **内容相似**：内容相似。常见的文本相似性算法有simhash等；
* **标题相似**：短文本相似性。从产品维度上，标题相似主要目的是去除同质内容，保证搜索结果的多角度展现。

## 6. 排序&截断
排序需要考虑多个维度：
* **相关性**：必须与用户需求相关；
* **权威性**：优先展示权威的结果，例如官网，或子领域里较权威的站点；
* **时效性**：同等条件下，优先展示新生产的内容。在资讯需求下尤甚，比如，2019年搜“nba总决赛”，出2018年总决赛的新闻肯定不合适；
* **页面质量**：比如图文并茂的内容更能吸引人、再比如内容空短、空洞乏味、内容段落重复、图片死链、广告、软文、蹭热点、诱导下载等等，都会影响用户体验。

从用户体验上来讲，除了上述基础特征外，还要考虑一些负向特征，对其进行打压。包括：
* **作弊**：页面作弊，这里不展开；
* **广告**：主要指影响用户体验的广告；
* **死链**：页面被发布者删除；
* **低质**：这里指滥竽充数的站点，比如内容段落与段落之间重复、文章排版混乱等；
* ...

# 7. 结果展现
* 计算摘要
* 飘红
* ...


# References & 扩展阅读

https://en.wikipedia.org/wiki/Tf%E2%80%93idf

https://en.wikipedia.org/wiki/Okapi_BM25

https://en.wikipedia.org/wiki/Vector_space_model

https://en.wikipedia.org/wiki/Language_model

https://segmentfault.com/a/1190000004958644

https://segmentfault.com/a/1190000005270047

https://segmentfault.com/a/1190000005569529

http://www.cnblogs.com/changxiaoxiao/archive/2013/03/27/2984724.html

http://www.infoq.com/cn/articles/yhd-11-11-mlr-ec-search

http://www.infoq.com/cn/articles/a-search-engine-scheduling-architecture-for-reference

http://blog.csdn.net/heiyeshuwu/article/details/43429447
