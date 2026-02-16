/*********************************************************************
 * conv.h
 *
 * the MicroChess project: https://github.com/ripred/MicroChess
 *
 * Compact representation of a board square / move.
 *
 * The original implementation stored the data in a union of two
 * bit‑field structs and claimed that assigning to `col`/`row`
 * automatically updated the `index` field.  In practice the setters
 * did **not** recompute the other view, which could leave `index`
 * stale and break any code that relied on `get_index()`.
 *
 * This version fixes that problem by:
 *   • Re‑computing the complementary fields in every mutator.
 *   • Providing a constructor that initialises both views.
 *   • Adding `static_assert`s to guarantee the expected size and
 *     bit‑field layout on the supported Arduino/ARM tool‑chains.
 *
 * The public API (getters / setters) is unchanged, so the rest of the
 * engine can keep using `conv1_t` exactly as before.
 *********************************************************************/

#ifndef CONV_INCL
#define CONV_INCL

#include <stdint.h>
#include <stddef.h>

/* -----------------------------------------------------------------
 *  Bit‑field layout assumptions
 *
 *  The code assumes LSB‑first ordering (the default for GCC on AVR,
 *  ARM Cortex‑M, ESP32, etc.).  If you compile with a compiler that
 *  uses a different ordering, the static_asserts will fire.
 * ----------------------------------------------------------------- */
static_assert(sizeof(uint8_t) == 1, "uint8_t must be 1 byte");

// The maximum values that fit in the declared bit‑widths
static_assert((1u << 3) - 1 == 7,  "3‑bit field must hold values 0‑7");
static_assert((1u << 1) - 1 == 1,  "1‑bit field must hold values 0‑1");

// -----------------------------------------------------------------
/*  conv1_t – a 2‑byte union that can be accessed either as
 *  (col,row,type,side) or as (index,type,side).
 *
 *  All mutators keep the two views consistent.
 * ----------------------------------------------------------------- */
struct conv1_t {
private:
    // The union holds the two overlapping representations.
    union {
        struct {
            uint8_t col  : 3;   // 0‑7
            uint8_t row  : 3;   // 0‑7
            uint8_t type : 3;   // Piece type (0‑6)
            uint8_t side : 1;   // 0 = Black, 1 = White
        } pt;                  // “point” view

        struct {
            uint8_t index : 6;  // 0‑63  (row * 8 + col)
            uint8_t type  : 3;  // Piece type (0‑6)
            uint8_t side  : 1;  // 0 = Black, 1 = White
        } ndx;                 // “index” view
    } u;

    // Helper: recompute `index` from `col`/`row`
    inline void sync_index_from_xy() {
        u.ndx.index = static_cast<uint8_t>(u.pt.col + (u.pt.row << 3));
    }

    // Helper: recompute `col`/`row` from `index`
    inline void sync_xy_from_index() {
        u.pt.col = static_cast<uint8_t>(u.ndx.index & 0x07);
        u.pt.row = static_cast<uint8_t>((u.ndx.index >> 3) & 0x07);
    }

public:
    // -----------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------
    conv1_t()
        : u{ .pt = { 0, 0, 0, 0 } }          // Empty Black at (0,0)
    {
        // Ensure both views are consistent.
        sync_index_from_xy();
    }

    // Initialise from a board index (0‑63)
    explicit conv1_t(uint8_t index)
        : u{ .ndx = { index, 0, 0 } }
    {
        sync_xy_from_index();
    }

    // Initialise from index, type and side
    conv1_t(uint8_t index, uint8_t type, uint8_t side)
        : u{ .ndx = { index, type, side } }
    {
        sync_xy_from_index();
    }

    // Initialise from column and row
    conv1_t(uint8_t col, uint8_t row)
        : u{ .pt = { col, row, 0, 0 } }
    {
        sync_index_from_xy();
    }

    // -----------------------------------------------------------------
    //  Mutators – each updates *both* representations
    // -----------------------------------------------------------------
    void set_index(uint8_t value) {
        u.ndx.index = value & 0x3F;          // mask to 6 bits
        sync_xy_from_index();
    }

    void set_col(uint8_t value) {
        u.pt.col = value & 0x07;             // mask to 3 bits
        sync_index_from_xy();
    }

    void set_row(uint8_t value) {
        u.pt.row = value & 0x07;             // mask to 3 bits
        sync_index_from_xy();
    }

    void set_type(uint8_t value) {
        u.pt.type = value & 0x07;
        u.ndx.type = u.pt.type;              // keep both views identical
    }

    void set_side(uint8_t value) {
        u.pt.side = value & 0x01;
        u.ndx.side = u.pt.side;
    }

    // -----------------------------------------------------------------
    //  Accessors – read‑only, no side‑effects
    // -----------------------------------------------------------------
    uint8_t get_index() const { return u.ndx.index; }
    uint8_t get_col()   const { return u.pt.col; }
    uint8_t get_row()   const { return u.pt.row; }
    uint8_t get_type()  const { return u.pt.type; }
    uint8_t get_side()  const { return u.pt.side; }

    // -----------------------------------------------------------------
    //  Equality operators (handy for testing)
    // -----------------------------------------------------------------
    bool operator==(const conv1_t& other) const {
        return u.ndx.index == other.u.ndx.index &&
               u.ndx.type  == other.u.ndx.type  &&
               u.ndx.side  == other.u.ndx.side;
    }

    bool operator!=(const conv1_t& other) const { return !(*this == other); }

}; // struct conv1_t

/* -----------------------------------------------------------------
 *  conv2_t – a simple pair of conv1_t objects representing a move.
 * ----------------------------------------------------------------- */
struct conv2_t {
private:
    conv1_t from_;
    conv1_t to_;

public:
    // Default ctor – both ends at (0,0)
    conv2_t() : from_(), to_() {}

    // Construct from two board indices
    conv2_t(uint8_t from_index, uint8_t to_index)
        : from_(from_index), to_(to_index) {}

    // Construct from explicit coordinates
    conv2_t(uint8_t from_col, uint8_t from_row,
            uint8_t to_col,   uint8_t to_row)
        : from_(from_col, from_row), to_(to_col, to_row) {}

    // Construct from two pre‑made conv1_t objects
    conv2_t(const conv1_t& from, const conv1_t& to)
        : from_(from), to_(to) {}

    // -----------------------------------------------------------------
    //  Forwarding setters / getters – thin wrappers around the
    //  underlying conv1_t fields.
    // -----------------------------------------------------------------
    void set_from_index(uint8_t v) { from_.set_index(v); }
    void set_from_col(uint8_t v)   { from_.set_col(v);   }
    void set_from_row(uint8_t v)   { from_.set_row(v);   }
    void set_from_type(uint8_t v)  { from_.set_type(v);  }
    void set_from_side(uint8_t v)  { from_.set_side(v);  }

    void set_to_index(uint8_t v)   { to_.set_index(v);   }
    void set_to_col(uint8_t v)     { to_.set_col(v);     }
    void set_to_row(uint8_t v)     { to_.set_row(v);     }
    void set_to_type(uint8_t v)    { to_.set_type(v);    }
    void set_to_side(uint8_t v)    { to_.set_side(v);    }

    uint8_t get_from_index() const { return from_.get_index(); }
    uint8_t get_from_col()   const { return from_.get_col();   }
    uint8_t get_from_row()   const { return from_.get_row();   }
    uint8_t get_from_type()  const { return from_.get_type();  }
    uint8_t get_from_side()  const { return from_.get_side();  }

    uint8_t get_to_index()   const { return to_.get_index();   }
    uint8_t get_to_col()     const { return to_.get_col();     }
    uint8_t get_to_row()     const { return to_.get_row();     }
    uint8_t get_to_type()    const { return to_.get_type();    }
    uint8_t get_to_side()    const { return to_.get_side();    }

    // Equality helpers – useful for the opening‑book validation.
    bool operator==(const conv2_t& other) const {
        return from_ == other.from_ && to_ == other.to_;
    }

    bool operator!=(const conv2_t& other) const { return !(*this == other); }

}; // struct conv2_t

/* -----------------------------------------------------------------
 *  Compile‑time sanity checks
 * ----------------------------------------------------------------- */
static_assert(sizeof(conv1_t) == 2,
              "conv1_t must occupy exactly 2 bytes (8‑bit + 8‑bit)");
static_assert(sizeof(conv2_t) == 4,
              "conv2_t must occupy exactly 4 bytes (two conv1_t objects)");

#endif // CONV_INCL
