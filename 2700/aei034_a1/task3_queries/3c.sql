.mode column
.head on

select orderNumber, requiredDate, productName, quantityOrdered, quantityInStock
from Orders O
natural join Products P
natural join OrderDetails OD
where O.status like "%in process%";
