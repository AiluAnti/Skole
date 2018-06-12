.mode column
.head on

/* OPPGAVE 3E, samme som du viste oss i hjelpetime, men det var
    mer eller mindre bare en syntaksfeil*/
select customerNumber
from Customers C
where not exists
  (select productCode
  from Customers
    natural join Orders
    natural join OrderDetails
    natural join Products
    where customerNumber = 219
  except
    select productCode
    from Customers
    natural join Orders
    natural join OrderDetails
    natural join Products
    where customerNumber = C.customerNumber);
