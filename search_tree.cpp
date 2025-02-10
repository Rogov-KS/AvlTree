#include <iostream>
#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <queue>
#include <cmath>


template<typename T, bool IsMultiSet>
class TSearchTree {
  protected:
    struct Node {
        T value_;
        std::shared_ptr<Node> left_;
        std::shared_ptr<Node> right_;
        std::weak_ptr<Node> parent_;
        size_t cnt_ = 1;

        bool operator<(const Node& other) {
            return value_ < other.value_;
        }

        explicit Node() {}
        Node(const T& value) : value_(value) {}
        Node(const T& value, 
             std::shared_ptr<Node> left, 
             std::shared_ptr<Node> right,
             std::weak_ptr<Node> parent
            ) : value_(value), left_(left), right_(right), parent_(parent) {}
        
        virtual ~Node() {}
    };

  public:
    TSearchTree() {}

    friend std::ostream& operator<<(std::ostream& os, const TSearchTree<T, IsMultiSet>& tree) {
        // BFS
        std::queue<std::pair<std::shared_ptr<Node>, size_t>> queue;
        queue.push({tree.root_, 0});
        size_t cur_height = 0;
        bool is_live = true;

        std::cout << "root = " << tree.root_->value_ << "\n";

        while (!queue.empty()) {
            const auto& [node_ptr, node_height] = queue.front();
            queue.pop();

            if (node_height > cur_height) {
                if (!is_live) break;
                is_live = false;
                os << "\n";
                cur_height = node_height;
            }

            if (node_ptr) {
                auto help_info = tree.HelpOutput(node_ptr);
                os << node_ptr->value_ << "(" << help_info << ") | ";
                queue.push({node_ptr->left_, node_height+1});
                queue.push({node_ptr->right_, node_height+1});
                is_live = true;
            } else {
                os << "   | ";
                queue.push({nullptr, node_height+1});
                queue.push({nullptr, node_height+1});
            }
        }
        os << "\n";
        return os;
    }

    void Insert(const T& value) {
        root_ = InsertRecursive(value, root_);
        root_->parent_ = std::shared_ptr<Node>(nullptr);
    }
    void Erase(const T& value) {
        root_ = EraseRecursive(value, root_);
        root_->parent_ = std::shared_ptr<Node>(nullptr);;
    }
    bool Exsist(const T& value) const {
        return (bool)(FindRecursive(value, root_));
    }

    size_t Size() const {
        return size_;
    }
    bool Empty() const {
        return size_ == 0;
    }

  protected:
    static void SwapNodesValue(std::shared_ptr<Node> n1, std::shared_ptr<Node> n2) {
        std::swap(n1->value_, n2->value_);
        std::swap(n1->cnt_, n2->cnt_);
    }
    static bool IsLeftSon(std::shared_ptr<Node> cur_node) {
        auto parent = cur_node->parent_.lock();
        return parent && parent->left_ == cur_node;
    }
    static bool IsRightSon(std::shared_ptr<Node> cur_node) {
        auto parent = cur_node->parent_.lock();
        return parent && parent->right_ == cur_node;
    }

    virtual std::shared_ptr<Node> Balance(std::shared_ptr<Node> node) {
        return node;
    }
    virtual std::shared_ptr<Node> CreateNode(const T& value) {
        return std::make_shared<Node>(value);
    }

    std::shared_ptr<Node> InsertRecursive(const T& value, std::shared_ptr<Node> node) {
        if (!node) {
            size_ += 1;
            return CreateNode(value);
        } else if (node->value_ == value) {
            InsertIfFound(node);
        } else if (node->value_ > value) {
            node->left_ = InsertRecursive(value, node->left_);
            if (node->left_) node->left_->parent_ = node;
        } else { // node->value_ < value
            node->right_ = InsertRecursive(value, node->right_);
            if (node->right_) node->right_->parent_ = node;
        }
        return Balance(node);
    }
    void InsertIfFound(const std::shared_ptr<Node>& cur_node) {
        if (IsMultiSet) {
            cur_node->cnt_ += 1;
        }
    }

    std::shared_ptr<Node> EraseRecursive(const T& value, std::shared_ptr<Node> node) {
        if (!node) {
            return nullptr;
        } else if (node->value_ == value) {
            return EraseIfFound(node);
        } else if (node->value_ > value) {
            node->left_ = EraseRecursive(value, node->left_);
            if (node->left_) node->left_->parent_ = node;
            return node;
        } else { // node->value_ < value
            node->right_ = EraseRecursive(value, node->right_);
            if (node->right_) node->right_->parent_ = node;
            return node;
        }
    }
    std::shared_ptr<Node> EraseIfFound(std::shared_ptr<Node> node) {
        if (!node->left_ || !node->right_) {
            return node->left_ ? node->left_ : node->right_;
        } else {
            auto change_node = FindMin(node->right_);
            SwapNodesValue(node, change_node);
            node->right_ = EraseRecursive(change_node->value_, node->right_);
            if (node->right_) node->right_->parent_ = node;
            return node;
        }
    }

    std::shared_ptr<Node> FindRecursive(const T& value, std::shared_ptr<Node> node) const {
        if (!node) {
            return nullptr;
        } else if (node->value_ == value) {
            return node;
        } else if(node->value_ > value) {
            return FindRecursive(value, node->left_);
        } else { // node->value_ < value
            return FindRecursive(value, node->right_);
        }
    }
    
    std::shared_ptr<Node> CreateFakeNodeWithLeftSon(std::shared_ptr<Node> node) {
        auto new_node = std::make_shared<Node>(root_->value_);
        new_node->left_ = node;
        return new_node;
    }

    virtual std::string HelpOutput(std::shared_ptr<Node> node) const {
        auto parent_value = node->parent_.lock() ? std::to_string(node->parent_.lock()->value_) : "none";
        return parent_value;
    }

  public:
    template <bool IsConst>
    class BaseIterator {
      public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::conditional_t<IsConst, const T, T>;
        using difference_type = int64_t;
        using pointer = value_type*;
        using reference = value_type&;
    
        BaseIterator(std::shared_ptr<Node> node) : node_ptr(node) {}
        
        reference operator*() const { return node_ptr->value_; }
        pointer operator->() { return &(node_ptr->value_); }

        // Префиксные
        BaseIterator& operator++() { 
            if (node_ptr->right_) {
                node_ptr = FindMin(node_ptr->right_);
            } else {
                while (IsRightSon(node_ptr)) {
                    node_ptr = node_ptr->parent_.lock();
                }
                node_ptr = node_ptr->parent_.lock();
            }
            return *this;
        }  
        BaseIterator& operator--() { 
            if (node_ptr->left_) {
                node_ptr = FindMax(node_ptr->left_);
            } else {
                while (IsLeftSon(node_ptr)) {
                    node_ptr = node_ptr->parent_.lock();
                }
                node_ptr = node_ptr->parent_.lock();
            }
            return *this; 
        }  

        // Постфиксные
        BaseIterator operator++(int) {
            BaseIterator tmp = *this; 
            ++(*this); 
            return tmp; 
        }
        BaseIterator operator--(int) {
            BaseIterator tmp = *this; 
            --(*this); 
            return tmp; 
        }

        bool IsValid() const {
            return (bool)node_ptr;
        }
        friend bool operator==(const BaseIterator& a, const BaseIterator& b) { return a.node_ptr == b.node_ptr; }
        friend bool operator!=(const BaseIterator& a, const BaseIterator& b) { return a.node_ptr != b.node_ptr; }
        friend bool operator<(const BaseIterator& a, const BaseIterator& b) { return *a.node_ptr < *b.node_ptr; }
        friend bool operator>(const BaseIterator& a, const BaseIterator& b) { return *a.node_ptr > *b.node_ptr; }
        friend bool operator<=(const BaseIterator& a, const BaseIterator& b) { return *a.node_ptr <= *b.node_ptr; }
        friend bool operator>=(const BaseIterator& a, const BaseIterator& b) { return *a.node_ptr >= *b.node_ptr; }
      
      protected:
        std::shared_ptr<Node> node_ptr;
    };
    
    using Iterator = BaseIterator<true>;
    // using Iterator = BaseIterator<true> - | usually, but we cant modify nodes in SearchTree
    using ConstIterator = BaseIterator<true>;
    using ReverseIterator = std::reverse_iterator<Iterator>;
    using ConstReverseIterator = std::reverse_iterator<ConstIterator>;
 
    Iterator Begin() {
        return Iterator(FindMin(root_));
    }
    Iterator End() {
        return Iterator(nullptr);
    }
    ConstIterator Begin() const {
        return ConstIterator(FindMin(root_));
    }
    ConstIterator End() const {
        return ConstIterator(nullptr);
    }
    Iterator begin() {
        return Begin();
    }
    Iterator end() {
        return End();
    }
    ConstIterator begin() const {
        return Begin();
    }
    ConstIterator end() const {
        return End();
    }

    ConstIterator CBegin() {
        return ConstIterator(FindMin(root_));
    }
    ConstIterator CEnd() {
        return ConstIterator(nullptr);
    }

    ReverseIterator RBegin() {
        return ReverseIterator(CreateFakeNodeWithLeftSon(FindMax(root_)));
    }
    ReverseIterator REnd() {
        return ReverseIterator(Begin());
    }
    ConstReverseIterator RBegin() const {
        return ConstReverseIterator(CreateFakeNodeWithLeftSon(FindMax(root_)));
    }
    ConstReverseIterator REnd() const {
        return ConstReverseIterator(Begin());
    }
    ConstReverseIterator CRBegin() {
        return ConstReverseIterator(CreateFakeNodeWithLeftSon(FindMax(root_)));
    }
    ConstReverseIterator CREnd() {
        return ConstReverseIterator(CBegin());
    }


    Iterator Find(const T& value) const {
        return Iterator(FindRecursive(value, root_));
    }

    Iterator Next(const T& value) const {
        std::shared_ptr<Node> next;
        auto cur_node = root_;
        while(cur_node) {
            if (cur_node->value_ > value) {
                if (!next || (cur_node->value_ < next->value_)) {
                    next = cur_node;
                } 
                
                cur_node = cur_node->left_;
            } else { // cur_node->value_ <= value
                cur_node = cur_node->right_;
            }
        }
        return Iterator(next);
    }
    Iterator Prev(const T& value) const {
        std::shared_ptr<Node> prev;
        auto cur_node = root_;
        while(cur_node) {
            if (cur_node->value_ < value) {
                if (!prev || (cur_node->value_ > prev->value_)) {
                    prev = cur_node;
                } 
                
                cur_node = cur_node->right_;
            } else { // cur_node->value_ >= value
                cur_node = cur_node->left_;
            }
        }
        return Iterator(prev);
    }

  protected:
    static std::shared_ptr<Node> FindMin(std::shared_ptr<Node> node) {
        while (node && node->left_) {
            node = node->left_;
        }
        return node;
    }
    static std::shared_ptr<Node> FindMax(std::shared_ptr<Node> node) {
        while (node && node->right_) {
            node = node->right_;
        }
        return node;
    }

    std::shared_ptr<Node> root_ = nullptr;
    size_t size_ = 0;
};


template<typename T, bool IsMultiSet>
class TAvlTree : public TSearchTree<T, IsMultiSet> {
  private:
    using Node = typename TSearchTree<T, IsMultiSet>::Node;
  protected:
    struct AvlNode : Node {
        size_t height_ = 1;

        std::shared_ptr<AvlNode> left() const {
            return std::dynamic_pointer_cast<AvlNode>(this->left_);
        }
        std::shared_ptr<AvlNode> right() const {
            return std::dynamic_pointer_cast<AvlNode>(this->right_);
        }
        std::shared_ptr<AvlNode> parent() const {
            return std::dynamic_pointer_cast<AvlNode>(this->parent_.lock());
        }
        
        bool operator<(const Node& other) {
            return this->value_ < other.value_;
        }

        explicit AvlNode(): Node() {}
        AvlNode(const T& value, size_t height=1) : Node(value), height_(height) {}
    };

    std::shared_ptr<AvlNode> BalanceNode(std::shared_ptr<AvlNode> node) {
        if (!node)
            return nullptr;
        RenewNodesHeight(node);

        auto node_height = HeightDiff(node);

        if (node_height < -1) {
            return LeftRotation(node);
        } else if (node_height > 1) {
            return  RightRotation(node);
        } else {
            return node;
        }
    }

    size_t NodeHeight(std::shared_ptr<AvlNode> node) const {
        return (node ? node->height_ : 0);
    }
    int HeightDiff(std::shared_ptr<AvlNode> node) const {
        return node ? NodeHeight(node->left()) - NodeHeight(node->right()) : 0;
    }

  private:
    std::shared_ptr<Node> Balance(std::shared_ptr<Node> node) override {
        return BalanceNode(std::dynamic_pointer_cast<AvlNode>(node));
    }
    std::shared_ptr<Node> CreateNode(const T& value) override {
        return std::make_shared<AvlNode>(value);
    }

    std::string HelpOutput(std::shared_ptr<Node> node) const override {
        auto same_node = std::dynamic_pointer_cast<AvlNode>(node);
        auto height = same_node ? std::to_string(same_node->height_) : "none";
        return height;
    }

    virtual void RenewNodesHeight(std::shared_ptr<AvlNode> node) {
        if (node) {
            node->height_ = std::max(NodeHeight(node->right()), NodeHeight(node->left())) + 1;
        }
    }

    std::shared_ptr<AvlNode> RotateRight(std::shared_ptr<AvlNode> node) {
        std::shared_ptr<AvlNode> son = node->left();
        std::shared_ptr<AvlNode> T2 = son->right();

        son->right_ = node;
        node->left_ = T2;

        node->parent_ = son;
        if (T2) T2->parent_ = node;

        RenewNodesHeight(node);
        RenewNodesHeight(son);

        return son;
    }

    std::shared_ptr<AvlNode> RotateLeft(std::shared_ptr<AvlNode> node) {
        std::shared_ptr<AvlNode> son = node->right();
        std::shared_ptr<AvlNode> T2 = son->left();

        son->left_ = node;
        node->right_ = T2;

        node->parent_ = son;
        if (T2) T2->parent_ = node;

        RenewNodesHeight(node);
        RenewNodesHeight(son);

        return son;
    }

    std::shared_ptr<AvlNode> RightRotation(std::shared_ptr<AvlNode> node) {
        if (HeightDiff(node->left()) >= 0) {
            return RotateRight(node);
        } else {
            node->left_ = RotateLeft(node->left());
            if (node->left_) node->left_->parent_ = node;
            return RotateRight(node);
        }
    }
    std::shared_ptr<AvlNode> LeftRotation(std::shared_ptr<AvlNode> node) {
        if (HeightDiff(node->right()) <= 0) {
            return RotateLeft(node);
        } else {
            node->right_ = RotateRight(node->right());
            if (node->right_) node->right_->parent_ = node;
            return RotateLeft(node);
        }
    }

};


template<typename T, bool IsMultiSet>
class TAvlTreeWithSize : public TAvlTree<T, IsMultiSet> {
  private:
    using Node = typename TSearchTree<T, IsMultiSet>::Node;
    using AvlNode = typename TAvlTree<T, IsMultiSet>::AvlNode;
  protected:
    struct AvlNodeWithSize : AvlNode {
        AvlNodeWithSize(const T& value) : AvlNode(value) {}

        std::shared_ptr<AvlNodeWithSize> right() {
            return std::dynamic_pointer_cast<AvlNodeWithSize>(this->right_);
        }
        std::shared_ptr<AvlNodeWithSize> left() {
            return std::dynamic_pointer_cast<AvlNodeWithSize>(this->left_);
        }

        size_t size_ = 1;
    };

  private:
    std::shared_ptr<Node> CreateNode(const T& value) override {
        return std::make_shared<AvlNodeWithSize>(value);
    }
    std::string HelpOutput(std::shared_ptr<Node> node) const override {
        auto same_node = std::dynamic_pointer_cast<AvlNodeWithSize>(node);
        auto height = same_node ? std::to_string(same_node->size_) : "none";
        return height;
    }

    static size_t NodeSize(std::shared_ptr<AvlNodeWithSize> node) {
        return (node ? node->size_ : 0);
    }
    void RenewNodesHeight(std::shared_ptr<AvlNode> node) override {
        if (node) {
            auto same_node = std::dynamic_pointer_cast<AvlNodeWithSize>(node);
            same_node->height_ = std::max(this->NodeHeight(same_node->right()), this->NodeHeight(same_node->left())) + 1;
            same_node->size_ = (NodeSize(same_node->right()) + NodeSize(same_node->left())) + 1;
        }
    }

  public:
    template <bool IsConst>
    class BaseIterator : TSearchTree<T, IsMultiSet>::BaseIterator<IsConst> {
      private:
        using ParentClass = typename TSearchTree<T, IsMultiSet>::BaseIterator<IsConst>;
        using difference_type = typename ParentClass::difference_type;

      public:
        BaseIterator(std::shared_ptr<Node> node) : ParentClass(node) {}

        // BaseIterator operator+=(difference_type n) {
            // auto cur_n = n;
            // while (cur_n) {
            //     auto same_node = std::dynamic_pointer_cast<AvlNodeWithSize>(this->node_ptr);
            //     if (this->node_ptr->right_) {
            //         auto right_size = NodeSize(same_node->right());
            //         if (this->IsLeftSon(same_node) && right_size < cur_n) {
            //             cur_n -= right_size;
            //             this->node_ptr = this->node_ptr->parent_.lock();
            //         } else {
            //             this->node_ptr = this->FindMin(this->node_ptr->right_);
            //         }
            //     } else {
            //         while (this->IsRightSon(this->node_ptr)) {
            //             this->node_ptr = this->node_ptr->parent_.lock();
            //         }
            //         this->node_ptr = this->node_ptr->parent_.lock();
            //     }
            //     cur_n -= 1;
            // }

            // return *this;
        // }

        // friend BaseIterator operator+(const BaseIterator& it, difference_type n) {
        //     auto tmp(it);
        //     tmp += n;
        //     return tmp;
        // }

    };

    using Iterator = BaseIterator<true>;
    // using Iterator = BaseIterator<true> - | usually, but we cant modify nodes in SearchTree
    using ConstIterator = BaseIterator<true>;
    using ReverseIterator = std::reverse_iterator<Iterator>;
    using ConstReverseIterator = std::reverse_iterator<ConstIterator>;

    Iterator Begin() {
        return Iterator(this->FindMin(this->root_));
    }
    Iterator End() {
        return Iterator(nullptr);
    }

};


void main1() {

    std::cout << "Hello world, from binary heap\n";
    // TSearchTree<int64_t, false> tree;
    // TAvlTree<int64_t, false> tree;
    TAvlTreeWithSize<int64_t, false> tree;

    tree.Insert(10);
    tree.Insert(2);
    tree.Insert(6);
    tree.Insert(8);
    tree.Insert(3);

    tree.Insert(6);
    tree.Insert(1);
    tree.Insert(9);
    tree.Insert(3);
    tree.Insert(15);
    tree.Insert(13);
    tree.Insert(11);
    tree.Insert(12);
    tree.Insert(18);

    auto res = tree.Prev(8);
    std::cout << "res = " << *res << "\n";
    std::cout << tree;

    std::cout << "Iterating:\n";
    // for (auto it = tree.Begin(); it != tree.End(); ++it) {
    //     std::cout << *it << " | ";
    // }
    auto it1 = tree.Begin();
    // auto it2 = it1 + 4;
    // auto it3 = it2 + 3;
    // auto it = ++it2;
    
    std::cout << "\n";
    // std::cout << tree << "\n";
}

void main2() {

    std::ios_base::sync_with_stdio(0);
    std::cin.tie(0);
    std::cout.tie(0);

    // TSearchTree<int64_t, false> searchTree;
    TAvlTree<int64_t, false> searchTree;
    std::string s;
    int64_t x;
    auto res = searchTree.Next(0);
    while (std::cin >> s) {
        std::cin >> x;
        if (s == "insert") {
            searchTree.Insert(x);
        } else if (s == "delete") {
            searchTree.Erase(x);
        } else if (s == "exists") {
            std::cout << (searchTree.Exsist(x) ? "true" : "false") << "\n";
        } else if (s == "next") {
            res = searchTree.Next(x);
            if (res.IsValid()) {
                std::cout << (*res) << "\n";
            } else {
                std::cout << "none" << "\n";
            }
        } else if (s == "prev") {
            res = searchTree.Prev(x);
            if (res.IsValid()) {
                std::cout << (*res) << "\n";
            } else {
                std::cout << "none" << "\n";
            }
        } else {
            continue;
        }
    }

    // std::cout << "\n" << searchTree << "\n";
}

int main() {

    main1();

}