# 阿里云ODPS Map-Reduce
开发环境有源表orders： CREATE TABLE orders(order_id INT, customer_id STRING, employee_id INT, order_date STRING);
目标表gby_out：
CREATE TABLE gby_out(customer_id STRING, cnt INT);
请用ODPS MR实现以下SQL：
INSERT OVERWRITE TABLE gby_out
SELECT customer_id
, count(*) cnt
FROM orders
GROUP BY customer_id
HAVING cnt >= 3;

开发环境有源表orders和customers：
CREATE TABLE orders(order_id INT, customer_id STRING, employee_id INT, order_date STRING);
CREATE TABLE customers(customer_id STRING, customer_name STRING, address STRING, city STRING, country STRING);
目标表join_out：
CREATE TABLE join_out (customer_name STRING,order_id INT);
请用ODPS MR实现以下SQL：
INSERT OVERWRITE TABLE join_out
SELECT c.customer_name, o.order_id
FROM customers c
LEFT OUTER JOIN orders o
ON c.customer_id = o.customer_id AND c.country = 'UK';