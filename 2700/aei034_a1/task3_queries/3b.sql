.mode column
.head on

select productScale as Scale
from products
where productLine
like "%classic%";
