#create database and change focus on it
create database dust;
show databases;
use dust;

#create user and grant all privileges on tables inside dust database
create user 'username'@'localhost' identified by 'password';
grant all PRIVILEGES on dust.* to 'username'@'localhost';
select * from mysql.user;

#create table to grab all data
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
