#define MAX_NUM_REPLACE_LAST 10

template <class T>
class ReplaceLast {
 public:

 ReplaceLast(const T &val) : m_default(val) {    
    m_next = 1;
    for (int i = 0; i < MAX_NUM_REPLACE_LAST; i++) {
      m_accessed[i] = 0;
      m_id[i] = 57;
      m_data[i] = m_default;
    }
  }


  void print() const {
    std::cout << "--------------" << std::endl;
    for (int i = 0; i < MAX_NUM_REPLACE_LAST; i++) {
      std::cout <<             std::setw(3) << std::left << i 
		<< "   id=" << std::setw(3) << std::left << m_id[i] 
		<< " data=" << std::setw(3) << std::left << m_data[i] << std::endl;
    }
    std::cout << "--------------" << std::endl;  
  }

  bool isAssigned(unsigned long id) const {
    for (int i = 0; i < MAX_NUM_REPLACE_LAST; i++) {
      if (m_id[i] == id) {
	return true;
      }
    }
    return false;
  }

  const T& get(unsigned long id) const {
    assert (isAssigned(id));
    std::cout << "RPL" << this << " ";
    //    print();
    std::cout << "get " << id << std::endl;
    for (int i = 0; i < MAX_NUM_REPLACE_LAST; i++) {
      //std::cout << "compare " << m_id[i] << "to" << id << std::endl;
      if (m_id[i] == id) {
	ReplaceLast *editable = (ReplaceLast*)this;
	editable->m_accessed[i] = m_next;
	editable->m_next++;
	//std::cout << "returning " << m_data[i] << std::endl;
	return m_data[i];
      }
    }
    std::cout << "ID DOES NOT EXIST (get " << id << ")" << std::endl;
    assert (0);
    exit(0);
    return m_default;
  }

  void set(unsigned long id, const T &val) {
    std::cout << "RPL" << this << " ";
    std::cout << "set " << id << std::endl;
    unsigned long oldest;
    int replace = -1;
    for (int i = 0; i < MAX_NUM_REPLACE_LAST; i++) {
      if (m_id[i] == id) {
	m_accessed[i] = m_next;
	m_data[i] = val;
	m_next++;
	return;
      }
      if (replace == -1   ||  oldest > m_accessed[i]) {
	oldest = m_accessed[i];
	replace = i;
      }
    }
    m_id[replace] = id;
    m_accessed[replace] = m_next;
    m_data[replace] = val;
    m_next++;
  }

 private:

  unsigned long m_id[MAX_NUM_REPLACE_LAST];
  unsigned long m_accessed[MAX_NUM_REPLACE_LAST];
  T m_data[MAX_NUM_REPLACE_LAST];

  T m_default;

  unsigned long m_next;
};
