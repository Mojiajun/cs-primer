# Pratical Network Programming -- ChenShuo

## Layered Network
- Ethernet frame：以太网帧
- IP packet：IP分组
- TCP segment：TCP分节
- Application message：应用消息

## 常见错误
- 网络IO和业务逻辑混杂，不利于测试与维护
- TCP数据不完整（断开时机不正确）
- TCP数据没有边界
- 直接发送结构体（对齐、不可扩展）
- TCP自连接
- 非阻塞IO编程太多坑

## Test TCP (ttcp)
- 测试TCP性能的工具
- 性能指标
  - 带宽（Bandwidth, MB/s）
  - 吞吐量（Troughput, message/s, queries/s{QPS}, transactions/s{TPS}）
  - 延迟（Latency, milliseconds, percentiles）
  - 资源使用率（Utilization, percent, payload vs. carrier, goodput vs. theory BW）
  - 额外开销（Overhand, CPU usage, for compression and/or encryption）