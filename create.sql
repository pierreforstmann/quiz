--
-- create.sql
--
-- create schema objects
--
drop table if exists a;
drop table if exists q;
create table q(
       id integer primary key autoincrement,
       question text not null);
--
create table a(
	id int references q,
        no int not null,
        answer text not null, 
	solution text  default 'n' check (solution = 'y' or solution = 'n'),
        constraint pk_a primary key(id,no));
--
.schema
--
.quit 
