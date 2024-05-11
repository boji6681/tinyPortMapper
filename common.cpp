/*
 * comm.cpp
 *
 *  Created on: Jul 29, 2017
 *      Author: wangyu
 */

#include "common.h"
#include "log.h"



int about_to_exit=0;


int socket_buf_size=1024*1024;



struct random_fd_t
{
	int random_number_fd;
	random_fd_t()
	{
			random_number_fd=open("/dev/urandom",O_RDONLY);

			if(random_number_fd==-1)
			{
				mylog(log_fatal,"error open /dev/urandom\n");
				myexit(-1);
			}
			setnonblocking(random_number_fd);
	}
	int get_fd()
	{
		return random_number_fd;
	}
}random_fd;

u64_t get_current_time()//ms
{
	timespec tmp_time;
	clock_gettime(CLOCK_MONOTONIC, &tmp_time);
	return ((u64_t)tmp_time.tv_sec)*1000llu+((u64_t)tmp_time.tv_nsec)/(1000*1000llu);
}

u64_t get_current_time_us()
{
	timespec tmp_time;
	clock_gettime(CLOCK_MONOTONIC, &tmp_time);
	return (uint64_t(tmp_time.tv_sec))*1000llu*1000llu+ (uint64_t(tmp_time.tv_nsec))/1000llu;
}

u64_t pack_u64(u32_t a,u32_t b)
{
	u64_t ret=a;
	ret<<=32u;
	ret+=b;
	return ret;
}
u32_t get_u64_h(u64_t a)
{
	return a>>32u;
}
u32_t get_u64_l(u64_t a)
{
	return (a<<32u)>>32u;
}

void write_u16(char * p,u16_t w)
{
	*(unsigned char*)(p + 1) = (w & 0xff);
	*(unsigned char*)(p + 0) = (w >> 8);
}
u16_t read_u16(char * p)
{
	u16_t res;
	res = *(const unsigned char*)(p + 0);
	res = *(const unsigned char*)(p + 1) + (res << 8);
	return res;
}

void write_u32(char * p,u32_t l)
{
	*(unsigned char*)(p + 3) = (unsigned char)((l >>  0) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >>  8) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 0) = (unsigned char)((l >> 24) & 0xff);
}
u32_t read_u32(char * p)
{
	u32_t res;
	res = *(const unsigned char*)(p + 0);
	res = *(const unsigned char*)(p + 1) + (res << 8);
	res = *(const unsigned char*)(p + 2) + (res << 8);
	res = *(const unsigned char*)(p + 3) + (res << 8);
	return res;
}

void write_u64(char * s,u64_t a)
{
	assert(0==1);
}
u64_t read_u64(char * s)
{
	assert(0==1);
	return 0;
}


char * my_ntoa(u32_t ip)
{
	in_addr a;
	a.s_addr=ip;
	return inet_ntoa(a);
}

u64_t get_true_random_number_64()
{
	u64_t ret;
	int size=read(random_fd.get_fd(),&ret,sizeof(ret));
	if(size!=sizeof(ret))
	{
		mylog(log_fatal,"get random number failed %d\n",size);

		myexit(-1);
	}

	return ret;
}
u32_t get_true_random_number()
{
	u32_t ret;
	int size=read(random_fd.get_fd(),&ret,sizeof(ret));
	if(size!=sizeof(ret))
	{
		mylog(log_fatal,"get random number failed %d\n",size);
		myexit(-1);
	}
	return ret;
}
u32_t get_true_random_number_nz() //nz for non-zero
{
	u32_t ret=0;
	while(ret==0)
	{
		ret=get_true_random_number();
	}
	return ret;
}
u64_t ntoh64(u64_t a)
{
	if(__BYTE_ORDER == __LITTLE_ENDIAN)
	{
		return __bswap_64( a);
	}
	else return a;

}
u64_t hton64(u64_t a)
{
	if(__BYTE_ORDER == __LITTLE_ENDIAN)
	{
		return __bswap_64( a);
	}
	else return a;

}

void setnonblocking(int sock) {
	int opts;
	opts = fcntl(sock, F_GETFL);

	if (opts < 0) {
    	mylog(log_fatal,"fcntl(sock,GETFL)\n");
		//perror("fcntl(sock,GETFL)");
		myexit(1);
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sock, F_SETFL, opts) < 0) {
    	mylog(log_fatal,"fcntl(sock,SETFL,opts)\n");
		//perror("fcntl(sock,SETFL,opts)");
		myexit(1);
	}

}

int set_buf_size(int fd,int socket_buf_size,int force_socket_buf)
{
	if(force_socket_buf)
	{
		if(setsockopt(fd, SOL_SOCKET, SO_SNDBUFFORCE, &socket_buf_size, sizeof(socket_buf_size))<0)
		{
			mylog(log_fatal,"SO_SNDBUFFORCE fail  socket_buf_size=%d  errno=%s\n",socket_buf_size,strerror(errno));
			myexit(1);
		}
		if(setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &socket_buf_size, sizeof(socket_buf_size))<0)
		{
			mylog(log_fatal,"SO_RCVBUFFORCE fail  socket_buf_size=%d  errno=%s\n",socket_buf_size,strerror(errno));
			myexit(1);
		}
	}
	else
	{
		if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &socket_buf_size, sizeof(socket_buf_size))<0)
		{
			mylog(log_fatal,"SO_SNDBUF fail  socket_buf_size=%d  errno=%s\n",socket_buf_size,strerror(errno));
			myexit(1);
		}
		if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &socket_buf_size, sizeof(socket_buf_size))<0)
		{
			mylog(log_fatal,"SO_RCVBUF fail  socket_buf_size=%d  errno=%s\n",socket_buf_size,strerror(errno));
			myexit(1);
		}
	}
	return 0;
}

void myexit(int a)
{
    if(enable_log_color)
   	 printf("%s\n",RESET);
   // clear_iptables_rule();
	exit(a);
}
void  signal_handler(int sig)
{
	about_to_exit=1;
    // myexit(0);
}

void get_true_random_chars(char * s,int len)
{
	int size=read(random_fd.get_fd(),s,len);
	if(size!=len)
	{
		printf("get random number failed\n");
		exit(-1);
	}
}

int random_between(u32_t a,u32_t b)
{
	if(a>b)
	{
		mylog(log_fatal,"min >max?? %d %d\n",a ,b);
		myexit(1);
	}
	if(a==b)return a;
	else return a+get_true_random_number()%(b+1-a);
}


int round_up_div(int a,int b)
{
	return (a+b-1)/b;
}

int create_fifo(char * file)
{
	if(mkfifo (file, 0666)!=0)
	{
		if(errno==EEXIST)
		{
			mylog(log_warn,"warning fifo file %s exist\n",file);
		}
		else
		{
			mylog(log_fatal,"create fifo file %s failed\n",file);
			myexit(-1);
		}
	}
	int fifo_fd=open (file, O_RDWR);
	if(fifo_fd<0)
	{
		mylog(log_fatal,"create fifo file %s failed\n",file);
		myexit(-1);
	}
	struct stat st;
	if (fstat(fifo_fd, &st)!=0)
	{
		mylog(log_fatal,"fstat failed for fifo file %s\n",file);
		myexit(-1);
	}

	if(!S_ISFIFO(st.st_mode))
	{
		mylog(log_fatal,"%s is not a fifo\n",file);
		myexit(-1);
	}

	setnonblocking(fifo_fd);
	return fifo_fd;
}


int new_listen_socket(int &fd,u32_t ip,int port)
{
	fd =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	int yes = 1;
	//setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

	struct sockaddr_in local_me={0};

	socklen_t slen = sizeof(sockaddr_in);
	//memset(&local_me, 0, sizeof(local_me));
	local_me.sin_family = AF_INET;
	local_me.sin_port = htons(port);
	local_me.sin_addr.s_addr = ip;

	if (bind(fd, (struct sockaddr*) &local_me, slen) == -1) {
		mylog(log_fatal,"socket bind error\n");
		//perror("socket bind error");
		myexit(1);
	}
	setnonblocking(fd);
    set_buf_size(fd,socket_buf_size);

    mylog(log_debug,"local_listen_fd=%d\n,",fd);

	return 0;
}

int set_timer(int epollfd,int &timer_fd)
{
	int ret;
	epoll_event ev;

	itimerspec its;
	memset(&its,0,sizeof(its));

	if((timer_fd=timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK)) < 0)
	{
		mylog(log_fatal,"timer_fd create error\n");
		myexit(1);
	}
	its.it_interval.tv_sec=(timer_interval/1000);
	its.it_interval.tv_nsec=(timer_interval%1000)*1000ll*1000ll;
	its.it_value.tv_nsec=1; //imidiately
	timerfd_settime(timer_fd,0,&its,0);
	setnonblocking(timer_fd);


	ev.events = EPOLLIN;
	ev.data.u64 = timer_fd;

	ret=epoll_ctl(epollfd, EPOLL_CTL_ADD, timer_fd, &ev);
	if (ret < 0) {
		mylog(log_fatal,"epoll_ctl return %d\n", ret);
		myexit(-1);
	}
	return 0;
}

int address_t::from_str(char *str)
{
	clear();

	char ip_addr_str[100];u32_t port;
	mylog(log_info,"parsing address: %s\n",str);
	int is_ipv6=0;
	if(sscanf(str, "[%[^]]]:%u", ip_addr_str,&port)==2)
	{
		mylog(log_info,"its an ipv6 adress\n");
		inner.ipv6.sin6_family=AF_INET6;
		is_ipv6=1;
	}
	else if(sscanf(str, "%[^:]:%u", ip_addr_str,&port)==2)
	{
		mylog(log_info,"its an ipv4 adress\n");
		inner.ipv4.sin_family=AF_INET;
	}
	else
	{
		mylog(log_error,"failed to parse\n");
		myexit(-1);
	}

	mylog(log_info,"ip_address is {%s}, port is {%u}\n",ip_addr_str,port);

	if(port>65535)
	{
		mylog(log_error,"invalid port: %d\n",port);
		myexit(-1);
	}

	int ret=-100;
	if(is_ipv6)
	{
		ret=inet_pton(AF_INET6, ip_addr_str,&(inner.ipv6.sin6_addr));
		inner.ipv6.sin6_port=htons(port);
		if(ret==0)  // 0 if address type doesnt match
		{
			mylog(log_error,"ip_addr %s is not an ipv6 address, %d\n",ip_addr_str,ret);
			myexit(-1);
		}
		else if(ret==1) // inet_pton returns 1 on success
		{
			//okay
		}
		else
		{
			mylog(log_error,"ip_addr %s is invalid, %d\n",ip_addr_str,ret);
			myexit(-1);
		}
	}
	else
	{
		ret=inet_pton(AF_INET, ip_addr_str,&(inner.ipv4.sin_addr));
		inner.ipv4.sin_port=htons(port);

		if(ret==0)
		{
			mylog(log_error,"ip_addr %s is not an ipv4 address, %d\n",ip_addr_str,ret);
			myexit(-1);
		}
		else if(ret==1)
		{
			//okay
		}
		else
		{
			mylog(log_error,"ip_addr %s is invalid, %d\n",ip_addr_str,ret);
			myexit(-1);
		}
	}

	return 0;
}

char * address_t::get_str()
{
	static char res[max_addr_len];
	to_str(res);
	return res;
}
void address_t::to_str(char * s)
{
	//static char res[max_addr_len];
	char ip_addr[max_addr_len];
	u32_t port;
	const char * ret=0;
	if(get_type()==AF_INET6)
	{
		ret=inet_ntop(AF_INET6, &inner.ipv6.sin6_addr, ip_addr,max_addr_len);
		port=inner.ipv6.sin6_port;
	}
	else if(get_type()==AF_INET)
	{
		ret=inet_ntop(AF_INET, &inner.ipv4.sin_addr, ip_addr,max_addr_len);
		port=inner.ipv4.sin_port;
	}
	else
	{
		assert(0==1);
	}

	if(ret==0) //NULL on failure
	{
		mylog(log_error,"inet_ntop failed\n");
		myexit(-1);
	}

	port=ntohs(port);

	ip_addr[max_addr_len-1]=0;
	if(get_type()==AF_INET6)
	{
		sprintf(s,"[%s]:%u",ip_addr,(u32_t)port);
	}else
	{
		sprintf(s,"%s:%u",ip_addr,(u32_t)port);
	}

	//return res;
}

int address_t::from_sockaddr(sockaddr * addr,socklen_t slen)
{
	memset(&inner,0,sizeof(inner));
	if(addr->sa_family==AF_INET6)
	{
		assert(slen==sizeof(sockaddr_in6));
		inner.ipv6= *( (sockaddr_in6*) addr );

	}
	else if(addr->sa_family==AF_INET)
	{
		assert(slen==sizeof(sockaddr_in));
		inner.ipv4= *( (sockaddr_in*) addr );
	}
	else
	{
		assert(0==1);
	}
	return 0;
}

int address_t::new_connected_udp_fd()
{

	int new_udp_fd;
	new_udp_fd = socket(get_type(), SOCK_DGRAM, IPPROTO_UDP);
	if (new_udp_fd < 0) {
		mylog(log_warn, "create udp_fd error\n");
		return -1;
	}
	setnonblocking(new_udp_fd);
	set_buf_size(new_udp_fd,socket_buf_size);

	mylog(log_debug, "created new udp_fd %d\n", new_udp_fd);
	int ret = connect(new_udp_fd, (struct sockaddr *) &inner, get_len());
	if (ret != 0) {
		mylog(log_warn, "udp fd connect fail %d %s\n",ret,strerror(errno));
		close(new_udp_fd);
		return -1;
	}

	return new_udp_fd;
}

u32_t djb2(unsigned char *str,int len)
{
	 u32_t hash = 5381;
     int c;
     int i=0;
    while(c = *str++,i++!=len)
    {
         hash = ((hash << 5) + hash)^c; /* (hash * 33) ^ c */
    }

     hash=htonl(hash);
     return hash;
 }

u32_t sdbm(unsigned char *str,int len)
{
     u32_t hash = 0;
     int c;
     int i=0;
	while(c = *str++,i++!=len)
	{
		 hash = c + (hash << 6) + (hash << 16) - hash;
	}
     //hash=htonl(hash);
     return hash;
 }

