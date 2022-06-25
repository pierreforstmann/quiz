--
-- quiz.sql
--
drop table a;
drop table q;
create table q(
       id integer primary key autoincrement,
       question text not null,
       solution int not null);
--
create table a(
	id int references q,
        no int not null,
        answer text not null, 
        constraint pk_a primary key(id,no));
--
.schema
--
.quit 
