#ifndef INDEXIT_DB_H
#define INDEXIT_DB_H

/*
 * Initial database schema plan for IndexIt.
 *
 * This file is only a schema sketch.
 * SQLite/database implementation will be added later.
 *
 * Planned table: documents
 * ------------------------
 * id              INTEGER PRIMARY KEY
 * path            TEXT NOT NULL
 * filename        TEXT NOT NULL
 * type            TEXT
 * size            INTEGER
 * modified_date   TEXT
 *
 * Planned table: terms
 * --------------------
 * id              INTEGER PRIMARY KEY
 * term            TEXT NOT NULL UNIQUE
 *
 * Future work may add a relationship/posting table to connect
 * terms with documents as the inverted index is implemented.
 */

#endif /* INDEXIT_DB_H */
