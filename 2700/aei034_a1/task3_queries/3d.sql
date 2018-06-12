.mode column
.head on

/*OPPGAVE 3D */


select C.customerName as CustomerName, C.creditLimit as CreditLimit, SUM(P.amount) as TotalAmount, T.TotalPrice, (T.TotalPrice - SUM(P.amount)) AS Difference
from Payments as P
join Customers as C on P.customerNumber = C.customerNumber
join (select O.customerNumber as customerNumber, SUM(OD.quantityOrdered * OD.priceEach) as TotalPrice
  from OrderDetails as OD
  join Orders as O on O.orderNumber = OD.orderNumber
  group by O.customerNumber) as T
  on T.customerNumber = P.customerNumber
group by P.customerNumber
having ROUND(TotalAmount, 2) + C.creditLimit < ROUND(T.TotalPrice, 2);
