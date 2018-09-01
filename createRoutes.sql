SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET default_tablespace = '';

SET default_with_oids = false;

CREATE TABLE public."Controls" (
    controls_id integer NOT NULL,
    controls double precision[],
    evaluated double precision[]
);


ALTER TABLE public."Controls" OWNER TO evie;

CREATE SEQUENCE public."Controls_controls_id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."Controls_controls_id_seq" OWNER TO evie;

ALTER SEQUENCE public."Controls_controls_id_seq" OWNED BY public."Controls".controls_id;

CREATE TABLE public."Fitness" (
    fitness_id integer NOT NULL,
    total_fitness double precision,
    track_fitness double precision,
    curve_fitness double precision,
    grade_fitness double precision,
    length_fitness double precision,
    generation_id integer
);


ALTER TABLE public."Fitness" OWNER TO evie;

CREATE SEQUENCE public."Fitness_fitness_id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."Fitness_fitness_id_seq" OWNER TO evie;

ALTER SEQUENCE public."Fitness_fitness_id_seq" OWNED BY public."Fitness".fitness_id;

CREATE TABLE public."Generation" (
    generation integer NOT NULL,
    controls_id integer,
    route_id integer,
    generation_id integer NOT NULL
);


ALTER TABLE public."Generation" OWNER TO evie;

CREATE SEQUENCE public."Generation_generation_id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."Generation_generation_id_seq" OWNER TO evie;

ALTER SEQUENCE public."Generation_generation_id_seq" OWNED BY public."Generation".generation_id;

CREATE TABLE public."Route" (
    lat_start double precision,
    lat_end double precision,
    long_start double precision,
    long_end double precision,
    route_id integer NOT NULL
);


ALTER TABLE public."Route" OWNER TO evie;

CREATE SEQUENCE public."Route_route_id_seq"
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public."Route_route_id_seq" OWNER TO evie;

ALTER SEQUENCE public."Route_route_id_seq" OWNED BY public."Route".route_id;

ALTER TABLE ONLY public."Controls" ALTER COLUMN controls_id SET DEFAULT nextval('public."Controls_controls_id_seq"'::regclass);

ALTER TABLE ONLY public."Fitness" ALTER COLUMN fitness_id SET DEFAULT nextval('public."Fitness_fitness_id_seq"'::regclass);

ALTER TABLE ONLY public."Generation" ALTER COLUMN generation_id SET DEFAULT nextval('public."Generation_generation_id_seq"'::regclass);

ALTER TABLE ONLY public."Route" ALTER COLUMN route_id SET DEFAULT nextval('public."Route_route_id_seq"'::regclass);

ALTER TABLE ONLY public."Controls"
    ADD CONSTRAINT controls_id PRIMARY KEY (controls_id);

ALTER TABLE ONLY public."Fitness"
    ADD CONSTRAINT fitness_id PRIMARY KEY (fitness_id);

ALTER TABLE ONLY public."Generation"
    ADD CONSTRAINT generation_id PRIMARY KEY (generation_id);

ALTER TABLE ONLY public."Route"
    ADD CONSTRAINT route_id PRIMARY KEY (route_id);

ALTER TABLE ONLY public."Generation"
    ADD CONSTRAINT controls_id FOREIGN KEY (controls_id) REFERENCES public."Controls"(controls_id);

ALTER TABLE ONLY public."Fitness"
    ADD CONSTRAINT generation_id FOREIGN KEY (generation_id) REFERENCES public."Generation"(generation_id);

ALTER TABLE ONLY public."Generation"
    ADD CONSTRAINT route_id FOREIGN KEY (route_id) REFERENCES public."Route"(route_id);

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM postgres;
GRANT ALL ON SCHEMA public TO postgres;
GRANT ALL ON SCHEMA public TO PUBLIC;
