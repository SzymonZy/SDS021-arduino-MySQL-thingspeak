create database dust;
show databases;
use dust;
grant all on dust.* to dust@'localhost' identified by 'password'; 
create table readings
(
     id 	int auto_increment primary key,
     time 	timestamp default now(), 
     pm25 	decimal(6,2),
     pm10 	decimal(6,2),
     heater 	int,
     RH   	decimal(6,2),
     temp	decimal(6,2)
);
desc readings;
select * from readings;
