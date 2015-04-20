CREATE TABLE library_path (
 id BIGINT AUTO_INCREMENT,
 path TEXT,
 PRIMARY KEY(id)
) ENGINE=InnoDB;

CREATE TABLE song (
 id BIGINT AUTO_INCREMENT,
 path BIGINT NOT NULL,
 artist TEXT,
 title TEXT,
 file TEXT,
 PRIMARY KEY(id),
 FOREIGN KEY(path) REFERENCES library_path(id)
) ENGINE=InnoDB;
